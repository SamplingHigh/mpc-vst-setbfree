/* setBfree -> mpc_engine_t adapter.
 *
 * The DSP is the upstream setBfree engine with its JACK/LV2 host layers removed.
 * MPC OS supplies 44.1 kHz audio in 128-frame blocks. Incoming channel messages
 * are normalized to setBfree's upper-manual channel; optional manual splits use
 * setBfree's own channel-A key map, so one keyboard can drive all three manuals.
 */
#define _GNU_SOURCE
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "global_inst.h"
#include "state.h"

#define BLOCK 128
#define PARAM_COUNT 44

double SampleRateD = 44100.0;

/* cfgParser.c expects these from a host front-end. This port has no host-level
 * configuration keys; all exposed controls are handled below. */
const ConfigDoc *mainDoc(void) { return NULL; }
int mainConfig(ConfigContext *cfg) { (void)cfg; return 0; }

typedef enum { SCALE_RAW, SCALE_DRAWBAR, SCALE_PERCENT, SCALE_TOGGLE,
               SCALE_VIBRATO, SCALE_ROTARY } scale_t;

typedef struct {
    const char *key;
    const char *control;
    int def;
    scale_t scale;
} param_def_t;

static const param_def_t defs[PARAM_COUNT] = {
    {"split_mode", NULL, 0, SCALE_RAW},
    {"pedal_split", NULL, 36, SCALE_RAW},
    {"upper_split", NULL, 60, SCALE_RAW},

    {"upper_16", "upper.drawbar16", 8, SCALE_DRAWBAR},
    {"upper_5_13", "upper.drawbar513", 8, SCALE_DRAWBAR},
    {"upper_8", "upper.drawbar8", 8, SCALE_DRAWBAR},
    {"upper_4", "upper.drawbar4", 0, SCALE_DRAWBAR},
    {"upper_2_23", "upper.drawbar223", 0, SCALE_DRAWBAR},
    {"upper_2", "upper.drawbar2", 0, SCALE_DRAWBAR},
    {"upper_1_35", "upper.drawbar135", 0, SCALE_DRAWBAR},
    {"upper_1_13", "upper.drawbar113", 0, SCALE_DRAWBAR},
    {"upper_1", "upper.drawbar1", 0, SCALE_DRAWBAR},

    {"lower_16", "lower.drawbar16", 8, SCALE_DRAWBAR},
    {"lower_5_13", "lower.drawbar513", 8, SCALE_DRAWBAR},
    {"lower_8", "lower.drawbar8", 8, SCALE_DRAWBAR},
    {"lower_4", "lower.drawbar4", 0, SCALE_DRAWBAR},
    {"lower_2_23", "lower.drawbar223", 0, SCALE_DRAWBAR},
    {"lower_2", "lower.drawbar2", 0, SCALE_DRAWBAR},
    {"lower_1_35", "lower.drawbar135", 0, SCALE_DRAWBAR},
    {"lower_1_13", "lower.drawbar113", 0, SCALE_DRAWBAR},
    {"lower_1", "lower.drawbar1", 0, SCALE_DRAWBAR},

    {"pedal_16", "pedal.drawbar16", 8, SCALE_DRAWBAR},
    {"pedal_5_13", "pedal.drawbar513", 8, SCALE_DRAWBAR},
    {"pedal_8", "pedal.drawbar8", 8, SCALE_DRAWBAR},
    {"pedal_4", "pedal.drawbar4", 0, SCALE_DRAWBAR},
    {"pedal_2_23", "pedal.drawbar223", 0, SCALE_DRAWBAR},
    {"pedal_2", "pedal.drawbar2", 0, SCALE_DRAWBAR},
    {"pedal_1_35", "pedal.drawbar135", 0, SCALE_DRAWBAR},
    {"pedal_1_13", "pedal.drawbar113", 0, SCALE_DRAWBAR},
    {"pedal_1", "pedal.drawbar1", 0, SCALE_DRAWBAR},

    {"percussion", "percussion.enable", 0, SCALE_TOGGLE},
    {"perc_volume", "percussion.volume", 1, SCALE_TOGGLE},
    {"perc_decay", "percussion.decay", 0, SCALE_TOGGLE},
    {"perc_harmonic", "percussion.harmonic", 1, SCALE_TOGGLE},
    {"vibrato", "vibrato.knob", 0, SCALE_VIBRATO},
    {"vibrato_upper", "vibrato.upper", 0, SCALE_TOGGLE},
    {"vibrato_lower", "vibrato.lower", 0, SCALE_TOGGLE},
    {"swell", "swellpedal1", 100, SCALE_PERCENT},
    {"rotary", "rotary.speed-preset", 1, SCALE_ROTARY},
    {"overdrive", "overdrive.enable", 0, SCALE_TOGGLE},
    {"drive_character", "overdrive.character", 47, SCALE_PERCENT},
    {"drive_input", "overdrive.inputgain", 16, SCALE_PERCENT},
    {"drive_output", "overdrive.outputgain", 47, SCALE_PERCENT},
    {"reverb", "reverb.mix", 10, SCALE_PERCENT},
};

typedef struct {
    b_instance core;
    float a[BLOCK], b[BLOCK], c[BLOCK], left[BLOCK], right[BLOCK];
    float horn[BLOCK], drum[BLOCK];
    int value[PARAM_COUNT];
} organ_t;

static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static int find_param(const char *key) {
    int i;
    for (i = 0; i < PARAM_COUNT; ++i)
        if (!strcmp(key, defs[i].key)) return i;
    return -1;
}

static unsigned char engine_value(const param_def_t *p, int v) {
    switch (p->scale) {
    case SCALE_DRAWBAR: return (unsigned char)((clampi(v, 0, 8) * 127 + 4) / 8);
    case SCALE_PERCENT: return (unsigned char)((clampi(v, 0, 100) * 127 + 50) / 100);
    case SCALE_TOGGLE: return v ? 127 : 0;
    case SCALE_VIBRATO: return (unsigned char)((clampi(v, 0, 6) * 127 + 3) / 6);
    case SCALE_ROTARY: return (unsigned char)(clampi(v, 0, 2) * 63 + (v >= 2));
    default: return (unsigned char)clampi(v, 0, 127);
    }
}

static void apply_split(organ_t *o) {
    int mode = clampi(o->value[0], 0, 2);
    int pedal = clampi(o->value[1], 24, 95);
    int upper = clampi(o->value[2], 25, 96);
    if (pedal >= upper) pedal = upper - 1;
    midi_panic(&o->core);
    if (mode == 0) setKeyboardSplitMulti(o->core.midicfg, 3, 0, 0, 0, 0, 0);
    else if (mode == 1) setKeyboardSplitMulti(o->core.midicfg, 3, 0, upper, 0, 0, 0);
    else setKeyboardSplitMulti(o->core.midicfg, 3, pedal, upper, 0, 0, 0);
}

static void apply_param(organ_t *o, int i) {
    if (i < 3) { apply_split(o); return; }
    callMIDIControlFunction(o->core.midicfg, defs[i].control,
                            engine_value(&defs[i], o->value[i]));
}

static void *organ_create(const char *data_dir) {
    (void)data_dir;
    organ_t *o = (organ_t *)calloc(1, sizeof(*o));
    if (!o) return NULL;
    o->core.state = allocRunningConfig();
    o->core.progs = allocProgs();
    o->core.reverb = allocReverb();
    o->core.whirl = allocWhirl();
    o->core.midicfg = allocMidiCfg(o->core.state);
    o->core.synth = allocTonegen();
    o->core.preamp = allocPreamp();
    if (!o->core.state || !o->core.progs || !o->core.reverb || !o->core.whirl ||
        !o->core.midicfg || !o->core.synth || !o->core.preamp) goto fail;

    initControllerTable(o->core.midicfg);
    midiPrimeControllerMapping(o->core.midicfg);
    initToneGenerator(o->core.synth, o->core.midicfg);
    initVibrato(o->core.synth, o->core.midicfg);
    initPreamp(o->core.preamp, o->core.midicfg);
    initReverb(o->core.reverb, o->core.midicfg, SampleRateD);
    initWhirl(o->core.whirl, o->core.midicfg, SampleRateD);
    initRunningConfig(o->core.state, o->core.midicfg);
    initMidiTables(o->core.midicfg);

    for (int i = 0; i < PARAM_COUNT; ++i) o->value[i] = defs[i].def;
    for (int i = 0; i < PARAM_COUNT; ++i) apply_param(o, i);
    return o;

fail:
    if (o->core.reverb) freeReverb(o->core.reverb);
    if (o->core.whirl) freeWhirl(o->core.whirl);
    if (o->core.synth) freeToneGenerator(o->core.synth);
    if (o->core.midicfg) freeMidiCfg(o->core.midicfg);
    if (o->core.preamp) freePreamp(o->core.preamp);
    if (o->core.progs) freeProgs(o->core.progs);
    if (o->core.state) freeRunningConfig(o->core.state);
    free(o);
    return NULL;
}

static void organ_destroy(void *instance) {
    organ_t *o = (organ_t *)instance;
    if (!o) return;
    midi_panic(&o->core);
    freeReverb(o->core.reverb);
    freeWhirl(o->core.whirl);
    freeToneGenerator(o->core.synth);
    freeMidiCfg(o->core.midicfg);
    freePreamp(o->core.preamp);
    freeProgs(o->core.progs);
    freeRunningConfig(o->core.state);
    free(o);
}

static void organ_midi(void *instance, const uint8_t *msg, int len) {
    organ_t *o = (organ_t *)instance;
    uint8_t m[3];
    if (!o || !msg || len < 2 || len > 3) return;
    memcpy(m, msg, (size_t)len);
    /* One-keyboard mode: setBfree channel A is MIDI channel 1 / nibble 0. */
    if ((m[0] & 0xf0) >= 0x80 && (m[0] & 0xf0) <= 0xe0) m[0] &= 0xf0;
    parse_raw_midi_data(&o->core, m, (size_t)len);
}

static void set_one(organ_t *o, const char *key, const char *text) {
    int i = find_param(key);
    if (i < 0) return;
    int v = (int)lround(strtod(text, NULL));
    if (i == 0) v = clampi(v, 0, 2);
    else if (i == 1 || i == 2) v = clampi(v, 0, 127);
    else if (defs[i].scale == SCALE_DRAWBAR) v = clampi(v, 0, 8);
    else if (defs[i].scale == SCALE_PERCENT) v = clampi(v, 0, 100);
    else if (defs[i].scale == SCALE_TOGGLE) v = !!v;
    else if (defs[i].scale == SCALE_VIBRATO) v = clampi(v, 0, 6);
    else if (defs[i].scale == SCALE_ROTARY) v = clampi(v, 0, 2);
    o->value[i] = v;
    apply_param(o, i);
}

static void organ_set_param(void *instance, const char *key, const char *value) {
    organ_t *o = (organ_t *)instance;
    if (!o || !key || !value) return;
    if (!strcmp(key, "state")) {
        char *copy = strdup(value), *save = NULL, *tok;
        if (!copy) return;
        midi_panic(&o->core);
        for (tok = strtok_r(copy, ";\n", &save); tok; tok = strtok_r(NULL, ";\n", &save)) {
            char *eq = strchr(tok, '=');
            if (eq) { *eq = 0; set_one(o, tok, eq + 1); }
        }
        free(copy);
        return;
    }
    set_one(o, key, value);
}

static int organ_get_param(void *instance, const char *key, char *buf, int n) {
    organ_t *o = (organ_t *)instance;
    if (!o || !key || !buf || n <= 0) return 0;
    if (!strcmp(key, "state")) {
        int used = 0;
        for (int i = 0; i < PARAM_COUNT && used < n; ++i) {
            int w = snprintf(buf + used, (size_t)(n - used), "%s%s=%d",
                             i ? ";" : "", defs[i].key, o->value[i]);
            if (w < 0 || w >= n - used) { buf[n - 1] = 0; return 0; }
            used += w;
        }
        return used;
    }
    static const char display_suffix[] = "_display";
    size_t key_len = strlen(key);
    size_t suffix_len = sizeof(display_suffix) - 1;
    if (key_len > suffix_len &&
        !strcmp(key + key_len - suffix_len, display_suffix)) {
        char raw_key[64];
        size_t raw_len = key_len - suffix_len;
        if (raw_len >= sizeof(raw_key)) return 0;
        memcpy(raw_key, key, raw_len);
        raw_key[raw_len] = 0;
        int display_param = find_param(raw_key);
        if (display_param < 0 || defs[display_param].scale != SCALE_DRAWBAR) return 0;
        return snprintf(buf, (size_t)n, "%d", 8 - o->value[display_param]);
    }
    int i = find_param(key);
    if (i < 0) return 0;
    return snprintf(buf, (size_t)n, "%d", o->value[i]);
}

static int16_t to_i16(float x) {
    if (x > 0.999969f) x = 0.999969f;
    if (x < -1.0f) x = -1.0f;
    return (int16_t)lrintf(x * 32768.0f);
}

static void organ_render(void *instance, int16_t *out, int frames) {
    organ_t *o = (organ_t *)instance;
    if (!o || !out || frames <= 0) return;
    while (frames > 0) {
        int count = frames > BLOCK ? BLOCK : frames;
        /* Tonegen is defined for full 128-sample fragments. The generic wrapper
         * always requests 128; the partial branch keeps the adapter defensive. */
        oscGenerateFragment(o->core.synth, o->a, BLOCK);
        preamp(o->core.preamp, o->a, o->b, BLOCK);
        reverb(o->core.reverb, o->b, o->c, BLOCK);
        whirlProc3(o->core.whirl, o->c, o->left, o->right,
                   o->horn, o->drum, BLOCK);
        for (int i = 0; i < count; ++i) {
            out[2 * i] = to_i16(o->left[i]);
            out[2 * i + 1] = to_i16(o->right[i]);
        }
        out += count * 2;
        frames -= count;
    }
}

static const mpc_engine_t engine = {
    organ_create, organ_destroy, organ_midi,
    organ_set_param, organ_get_param, organ_render
};

const mpc_engine_t *mpc_engine(void) { return &engine; }
