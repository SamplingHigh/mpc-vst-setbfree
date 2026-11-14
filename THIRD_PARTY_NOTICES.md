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

The build and skin generation use the MIT-licensed
[Open MPC porting kit](https://github.com/open-mpc/mpc-vst-plugins)
porting kit. The generic VST2 wrapper from that project is compiled into the
plugin. Release 0.3.1 uses Open MPC commit
`0dc17120cd7e952e95eeecdd820ac9e495a75ee9`. Releases 0.1.11 and 0.1.12 used
the earlier `setbfree-drawbar-support` work at commit
`66eca176028b5fd63622dd7769451d5e28eb11a1`.
