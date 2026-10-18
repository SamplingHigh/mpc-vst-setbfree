# Vendored setBfree source

Source: https://github.com/pantherb/setBfree

Commit: `2f41983b5b535bda9b1b1db38a06da29d50ff557`

Vendored on 2026-10-18. The `src`, `b_overdrive`, `b_reverb`, and `b_whirl`
engine directories are copied into `src/dsp`. JACK/ALSA standalone entry points
(`main.c`, `midi_aseq.c`, `midi_aseq.h`) are omitted. No DSP algorithm changes
have been made. `src/setbfree_engine.c` is the MPC-specific host adapter.

The cabinet convolution path is intentionally not included in this first port.
It adds external zita-convolver and libsndfile dependencies and is not part of
the setBfree LV2 synth's normal lightweight signal path.
