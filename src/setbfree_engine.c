/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * setBfree -> mpc_engine_t adapter.
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
#define PARAM_COUNT 55

double SampleRateD = 44100.0;

/* cfgParser.c expects these from a host front-end. This port has no host-level
 * configuration keys; all exposed controls are handled below. */
const ConfigDoc *mainDoc(void) { return NULL; }
int mainConfig(ConfigContext *cfg) { (void)cfg; return 0; }

typedef enum { SCALE_RAW, SCALE_DRAWBAR, SCALE_PERCENT, SCALE_TOGGLE,
               SCALE_VIBRATO, SCALE_ROTARY, SCALE_SECONDS, SCALE_UNIT,
               SCALE_SOFT_LEVEL, SCALE_PERC_GAIN, SCALE_MANUAL_VIBRATO,
               SCALE_CROSSTALK } scale_t;

typedef struct {
    const char *key;
    const char *control;
    double def;
    scale_t scale;
} param_def_t;

static const param_def_t defs[PARAM_COUNT] = {
    {"split_mode", NULL, 0, SCALE_RAW},
    {"pedal_split", NULL, 36, SCALE_RAW},
    {"upper_split", NULL, 60, SCALE_RAW},

    {"upper_16", "upper.drawbar16", 0, SCALE_DRAWBAR},
    {"upper_5_13", "upper.drawbar513", 0, SCALE_DRAWBAR},
    {"upper_8", "upper.drawbar8", 0, SCALE_DRAWBAR},
    {"upper_4", "upper.drawbar4", 8, SCALE_DRAWBAR},
    {"upper_2_23", "upper.drawbar223", 8, SCALE_DRAWBAR},
    {"upper_2", "upper.drawbar2", 8, SCALE_DRAWBAR},
    {"upper_1_35", "upper.drawbar135", 8, SCALE_DRAWBAR},
    {"upper_1_13", "upper.drawbar113", 8, SCALE_DRAWBAR},
    {"upper_1", "upper.drawbar1", 8, SCALE_DRAWBAR},

    {"lower_16", "lower.drawbar16", 0, SCALE_DRAWBAR},
    {"lower_5_13", "lower.drawbar513", 0, SCALE_DRAWBAR},
    {"lower_8", "lower.drawbar8", 0, SCALE_DRAWBAR},
    {"lower_4", "lower.drawbar4", 8, SCALE_DRAWBAR},
    {"lower_2_23", "lower.drawbar223", 8, SCALE_DRAWBAR},
    {"lower_2", "lower.drawbar2", 8, SCALE_DRAWBAR},
    {"lower_1_35", "lower.drawbar135", 8, SCALE_DRAWBAR},
    {"lower_1_13", "lower.drawbar113", 8, SCALE_DRAWBAR},
    {"lower_1", "lower.drawbar1", 8, SCALE_DRAWBAR},

    {"pedal_16", "pedal.drawbar16", 0, SCALE_DRAWBAR},
    {"pedal_5_13", "pedal.drawbar513", 0, SCALE_DRAWBAR},
    {"pedal_8", "pedal.drawbar8", 0, SCALE_DRAWBAR},
    {"pedal_4", "pedal.drawbar4", 8, SCALE_DRAWBAR},
    {"pedal_2_23", "pedal.drawbar223", 8, SCALE_DRAWBAR},
    {"pedal_2", "pedal.drawbar2", 8, SCALE_DRAWBAR},
    {"pedal_1_35", "pedal.drawbar135", 8, SCALE_DRAWBAR},
    {"pedal_1_13", "pedal.drawbar113", 8, SCALE_DRAWBAR},
    {"pedal_1", "pedal.drawbar1", 8, SCALE_DRAWBAR},

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

    {"perc_fast_time", NULL, 1.0, SCALE_SECONDS},
    {"perc_slow_time", NULL, 4.0, SCALE_SECONDS},
    {"perc_normal_level", NULL, 1.0, SCALE_UNIT},
    {"perc_soft_level", NULL, 0.5012, SCALE_SOFT_LEVEL},
    {"perc_gain", NULL, 11.0, SCALE_PERC_GAIN},
    {"vibrato_upper_mode", NULL, 3, SCALE_MANUAL_VIBRATO},
    {"vibrato_lower_mode", NULL, 3, SCALE_MANUAL_VIBRATO},
    {"compartment_crosstalk", NULL, 0.01, SCALE_CROSSTALK},
    {"transformer_crosstalk", NULL, 0.0, SCALE_CROSSTALK},
    {"terminal_crosstalk", NULL, 0.01, SCALE_CROSSTALK},
    {"wiring_crosstalk", NULL, 0.01, SCALE_CROSSTALK},
};

enum {
    P_SPLIT_MODE = 0, P_PEDAL_SPLIT = 1, P_UPPER_SPLIT = 2,
    P_PERC_VOLUME = 31, P_PERC_DECAY = 32, P_PERC_HARMONIC = 33,
    P_VIBRATO = 34, P_VIBRATO_UPPER = 35, P_VIBRATO_LOWER = 36,
    P_PERC_FAST_TIME = 44, P_PERC_SLOW_TIME, P_PERC_NORMAL_LEVEL,
    P_PERC_SOFT_LEVEL, P_PERC_GAIN, P_VIBRATO_UPPER_MODE,
    P_VIBRATO_LOWER_MODE, P_COMPARTMENT_CROSSTALK,
    P_TRANSFORMER_CROSSTALK, P_TERMINAL_CROSSTALK, P_WIRING_CROSSTALK
};

typedef struct {
    b_instance core;
    float a[BLOCK], b[BLOCK], c[BLOCK], left[BLOCK], right[BLOCK];
    float horn[BLOCK], drum[BLOCK];
    double value[PARAM_COUNT];
    int rebuild_delay;
    uint64_t changed;
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

static double clampd(double v, double lo, double hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static unsigned char engine_value(const param_def_t *p, double value) {
    int v = (int)lround(value);
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
    int mode = clampi((int)lround(o->value[P_SPLIT_MODE]), 0, 2);
    int pedal = clampi((int)lround(o->value[P_PEDAL_SPLIT]), 24, 95);
    int upper = clampi((int)lround(o->value[P_UPPER_SPLIT]), 25, 96);
    if (pedal >= upper) pedal = upper - 1;
    midi_panic(&o->core);
    if (mode == 0) setKeyboardSplitMulti(o->core.midicfg, 3, 0, 0, 0, 0, 0);
    else if (mode == 1) setKeyboardSplitMulti(o->core.midicfg, 3, 0, upper, 0, 0, 0);
    else setKeyboardSplitMulti(o->core.midicfg, 3, pedal, upper, 0, 0, 0);
}

static int old_vibrato_from_manual(int v) {
    static const int map[] = {5, 3, 1, 0, 2, 4, 6};
    return map[clampi(v, 0, 6)];
}

static int manual_vibrato_from_old(int v) {
    static const int map[] = {2, 2, 4, 1, 5, 0, 6};
    return map[clampi(v, 0, 6)];
}

static void mark_changed(organ_t *o, int i) {
    if (i >= 0 && i < 64) o->changed |= UINT64_C(1) << i;
}

static void apply_manual_vibrato(organ_t *o, int i) {
    int other = i == P_VIBRATO_UPPER_MODE ? P_VIBRATO_LOWER_MODE : P_VIBRATO_UPPER_MODE;
    int enable = i == P_VIBRATO_UPPER_MODE ? P_VIBRATO_UPPER : P_VIBRATO_LOWER;
    int mode = clampi((int)lround(o->value[i]), 0, 6);
    o->value[enable] = mode == 3 ? 0 : 1;
    callMIDIControlFunction(o->core.midicfg, defs[enable].control,
                            engine_value(&defs[enable], o->value[enable]));
    if (mode == 3) return;
    o->value[P_VIBRATO] = old_vibrato_from_manual(mode);
    callMIDIControlFunction(o->core.midicfg, defs[P_VIBRATO].control,
                            engine_value(&defs[P_VIBRATO], o->value[P_VIBRATO]));
    if ((int)lround(o->value[other]) != 3 && (int)lround(o->value[other]) != mode) {
        o->value[other] = mode;
        mark_changed(o, other);
    }
}

static void apply_param(organ_t *o, int i) {
    if (i < 3) { apply_split(o); return; }
    switch (i) {
    case P_PERC_VOLUME:
        setPercussionVolume(o->core.synth, (int)lround(o->value[i]) == 0);
        return;
    case P_PERC_DECAY:
        setPercussionFast(o->core.synth, (int)lround(o->value[i]) == 0);
        return;
    case P_PERC_HARMONIC:
        setPercussionFirst(o->core.synth, (int)lround(o->value[i]) == 0);
        return;
    case P_PERC_FAST_TIME:
        setFastPercussionDecay(o->core.synth, o->value[i]);
        return;
    case P_PERC_SLOW_TIME:
        setSlowPercussionDecay(o->core.synth, o->value[i]);
        return;
    case P_PERC_NORMAL_LEVEL:
        setNormalPercussionGain(o->core.synth, o->value[i]);
        setPercussionVolume(o->core.synth, (int)lround(o->value[P_PERC_VOLUME]) == 0);
        return;
    case P_PERC_SOFT_LEVEL:
        setSoftPercussionGain(o->core.synth, o->value[i]);
        setPercussionVolume(o->core.synth, (int)lround(o->value[P_PERC_VOLUME]) == 0);
        return;
    case P_PERC_GAIN:
        setPercussionGainScaling(o->core.synth, o->value[i]);
        setPercussionVolume(o->core.synth, (int)lround(o->value[P_PERC_VOLUME]) == 0);
        return;
    case P_VIBRATO_UPPER_MODE:
    case P_VIBRATO_LOWER_MODE:
        apply_manual_vibrato(o, i);
        return;
    case P_COMPARTMENT_CROSSTALK:
    case P_TRANSFORMER_CROSSTALK:
    case P_TERMINAL_CROSSTALK:
    case P_WIRING_CROSSTALK:
        /* These values are compiled into setBfree's play matrix. Wait a few
         * audio blocks after the last move so a touchscreen drag causes one
         * rebuild, rather than allocating a complete organ on every pixel. */
        o->rebuild_delay = 8;
        return;
    default:
        break;
    }
    if (defs[i].control)
        callMIDIControlFunction(o->core.midicfg, defs[i].control,
                                engine_value(&defs[i], o->value[i]));
}

static void free_core(b_instance *core) {
    if (!core) return;
    if (core->reverb) freeReverb(core->reverb);
    if (core->whirl) freeWhirl(core->whirl);
    if (core->synth) freeToneGenerator(core->synth);
    if (core->midicfg) freeMidiCfg(core->midicfg);
    if (core->preamp) freePreamp(core->preamp);
    if (core->progs) freeProgs(core->progs);
    if (core->state) freeRunningConfig(core->state);
    memset(core, 0, sizeof(*core));
}

static int init_core(organ_t *o, b_instance *core) {
    memset(core, 0, sizeof(*core));
    core->state = allocRunningConfig();
    core->progs = allocProgs();
    core->reverb = allocReverb();
    core->whirl = allocWhirl();
    core->midicfg = allocMidiCfg(core->state);
    core->synth = allocTonegen();
    core->preamp = allocPreamp();
    if (!core->state || !core->progs || !core->reverb || !core->whirl ||
        !core->midicfg || !core->synth || !core->preamp) {
        free_core(core);
        return 0;
    }
    core->synth->defaultCompartmentCrosstalk = o->value[P_COMPARTMENT_CROSSTALK];
    core->synth->defaultTransformerCrosstalk = o->value[P_TRANSFORMER_CROSSTALK];
    core->synth->defaultTerminalStripCrosstalk = o->value[P_TERMINAL_CROSSTALK];
    core->synth->defaultWiringCrosstalk = o->value[P_WIRING_CROSSTALK];
    initControllerTable(core->midicfg);
    midiPrimeControllerMapping(core->midicfg);
    initToneGenerator(core->synth, core->midicfg);
    initVibrato(core->synth, core->midicfg);
    initPreamp(core->preamp, core->midicfg);
    initReverb(core->reverb, core->midicfg, SampleRateD);
    initWhirl(core->whirl, core->midicfg, SampleRateD);
    initRunningConfig(core->state, core->midicfg);
    initMidiTables(core->midicfg);
    return 1;
}

static int rebuild_core(organ_t *o) {
    b_instance fresh, old;
    if (!init_core(o, &fresh)) return 0;
    old = o->core;
    o->core = fresh;
    o->rebuild_delay = 0;
    for (int i = 0; i < PARAM_COUNT; ++i)
        if (i < P_COMPARTMENT_CROSSTALK || i > P_WIRING_CROSSTALK) apply_param(o, i);
    midi_panic(&old);
    free_core(&old);
    return 1;
}

static void *organ_create(const char *data_dir) {
    (void)data_dir;
    organ_t *o = (organ_t *)calloc(1, sizeof(*o));
    if (!o) return NULL;
    for (int i = 0; i < PARAM_COUNT; ++i) o->value[i] = defs[i].def;
    if (!init_core(o, &o->core)) goto fail;
    for (int i = 0; i < PARAM_COUNT; ++i) apply_param(o, i);
    o->rebuild_delay = 0;
    return o;

fail:
    free_core(&o->core);
    free(o);
    return NULL;
}

static void organ_destroy(void *instance) {
    organ_t *o = (organ_t *)instance;
    if (!o) return;
    midi_panic(&o->core);
    free_core(&o->core);
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
    double v = strtod(text, NULL);
    if (i == 0) v = clampi((int)lround(v), 0, 2);
    else if (i == 1 || i == 2) v = clampi((int)lround(v), 0, 127);
    else if (defs[i].scale == SCALE_DRAWBAR) v = clampi((int)lround(v), 0, 8);
    else if (defs[i].scale == SCALE_PERCENT) v = clampi((int)lround(v), 0, 100);
    else if (defs[i].scale == SCALE_TOGGLE) v = lround(v) != 0;
    else if (defs[i].scale == SCALE_VIBRATO) v = clampi((int)lround(v), 0, 6);
    else if (defs[i].scale == SCALE_ROTARY) v = clampi((int)lround(v), 0, 2);
    else if (defs[i].scale == SCALE_SECONDS) v = clampd(v, 0.1, 10.0);
    else if (defs[i].scale == SCALE_UNIT || defs[i].scale == SCALE_CROSSTALK) v = clampd(v, 0.0, 1.0);
    else if (defs[i].scale == SCALE_SOFT_LEVEL) v = clampd(v, 0.0, 0.89125);
    else if (defs[i].scale == SCALE_PERC_GAIN) v = clampd(v, 0.0, 22.0);
    else if (defs[i].scale == SCALE_MANUAL_VIBRATO) v = clampi((int)lround(v), 0, 6);
    o->value[i] = v;
    apply_param(o, i);
    /* The published 0.1.12 project state contains the three original
     * vibrato parameters. Mirror them to the new manual selectors while
     * loading old projects; new state later overrides these compatibility
     * values with its explicit upper/lower mode keys. */
    if (i == P_VIBRATO) {
        int mode = manual_vibrato_from_old((int)lround(v));
        if ((int)lround(o->value[P_VIBRATO_UPPER])) {
            o->value[P_VIBRATO_UPPER_MODE] = mode;
            mark_changed(o, P_VIBRATO_UPPER_MODE);
        }
        if ((int)lround(o->value[P_VIBRATO_LOWER])) {
            o->value[P_VIBRATO_LOWER_MODE] = mode;
            mark_changed(o, P_VIBRATO_LOWER_MODE);
        }
    } else if (i == P_VIBRATO_UPPER || i == P_VIBRATO_LOWER) {
        int mode_i = i == P_VIBRATO_UPPER ? P_VIBRATO_UPPER_MODE : P_VIBRATO_LOWER_MODE;
        o->value[mode_i] = lround(v) ? manual_vibrato_from_old((int)lround(o->value[P_VIBRATO])) : 3;
        mark_changed(o, mode_i);
    }
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
            int w = snprintf(buf + used, (size_t)(n - used), "%s%s=%.9g",
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
        if (display_param < 0) return 0;
        if (defs[display_param].scale == SCALE_DRAWBAR)
            return snprintf(buf, (size_t)n, "%d", 8 - (int)lround(o->value[display_param]));
        if (defs[display_param].scale == SCALE_CROSSTALK)
            return snprintf(buf, (size_t)n, "%.3f", o->value[display_param]);
        return 0;
    }
    int i = find_param(key);
    if (i < 0) return 0;
    if (defs[i].scale == SCALE_SECONDS || defs[i].scale == SCALE_UNIT ||
        defs[i].scale == SCALE_SOFT_LEVEL || defs[i].scale == SCALE_PERC_GAIN ||
        defs[i].scale == SCALE_CROSSTALK)
        return snprintf(buf, (size_t)n, "%.5g", o->value[i]);
    return snprintf(buf, (size_t)n, "%d", (int)lround(o->value[i]));
}

int mpc_engine_changed(void *instance, char *key, int len) {
    organ_t *o = (organ_t *)instance;
    if (!o || !key || len <= 0 || !o->changed) return 0;
    int i = 0;
    while (i < PARAM_COUNT && !(o->changed & (UINT64_C(1) << i))) ++i;
    if (i >= PARAM_COUNT) { o->changed = 0; return 0; }
    o->changed &= ~(UINT64_C(1) << i);
    return snprintf(key, (size_t)len, "%s", defs[i].key);
}

static int16_t to_i16(float x) {
    if (x > 0.999969f) x = 0.999969f;
    if (x < -1.0f) x = -1.0f;
    return (int16_t)lrintf(x * 32768.0f);
}

static void organ_render(void *instance, int16_t *out, int frames) {
    organ_t *o = (organ_t *)instance;
    if (!o || !out || frames <= 0) return;
    if (o->rebuild_delay > 0 && --o->rebuild_delay == 0 && !rebuild_core(o))
        o->rebuild_delay = 8;
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
