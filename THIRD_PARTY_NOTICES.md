# Third-party notices

## setBfree

- Upstream project: <https://github.com/pantherb/setBfree>
- Vendored commit: `2f41983b5b535bda9b1b1db38a06da29d50ff557`
- Vendored location: `src/dsp`
- Licence: GNU General Public License, version 2 or any later version

The original copyright and licence notices in the vendored files are retained.
JACK/ALSA standalone entry points that the MPC plugin does not use were omitted;
the DSP algorithms were not changed. The complete list of omissions and the MPC
adapter boundary are documented in [`src/VENDORED.md`](src/VENDORED.md).

## MPC porting kit

The build and skin generation use
the MIT-licensed [mpc-vst-plugins](https://github.com/sd88me/mpc-vst-plugins)
porting kit. The generic VST2 wrapper from that project is compiled into the
plugin. Its source and the scripts used to produce the binary and skin are
publicly available in the
[SamplingHigh fork](https://github.com/SamplingHigh/mpc-vst-plugins). Release
0.1.11 was built from branch `setbfree-drawbar-support`, commit
`66eca176028b5fd63622dd7769451d5e28eb11a1`.
