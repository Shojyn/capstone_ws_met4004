# Changelog

All notable changes to this firmware are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project uses
pre-1.0 versioning: new features bump the minor version and fixes bump the patch
version.

Each entry corresponds to one push to `main` and one Git tag.

## [Unreleased]

## [0.3.0] - 2026-11-03

Firmware commits: `8075f2e..b3b18d7`

Build: Debug | Host tests: 2/2 pass | Hardware: flashed, behavior pending

### Added

- Linear actuator controller for PF2/PF3 with a 1 s timeout and a reversal pause.
- Manual control module for PF1 with 30 ms debounce and alternating direction.
- Host tests for actuator modes, button bounce, timeout, reversal pause, and tick
  wraparound.
- Firmware version, commit, and build type embedded in the ELF.
- GitHub Actions workflow that builds and runs the host tests.
- `docs/hardware.md` with the pin map, L298N wiring, and test checklist.

### Changed

- Root README updated from the LED baseline to the current valve and actuator
  behavior, with links to the changelog and hardware reference.

### Verified

- Firmware builds; FLASH 20080 B, RAM 3736 B.
- `ctest` passes 2/2.
- Embedded version string is present in the ELF.

### Pending hardware verification

- First press extends, timeout stops, next press retracts, press-to-stop.
- Valve cycles and LD1 closing blink remain correct.
- Embedded version string matches the tagged version.

[Unreleased]: https://github.com/Shojyn/capstone_ws_met4004/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/Shojyn/capstone_ws_met4004/releases/tag/v0.3.0
