# Changelog

All notable changes to this firmware are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project uses
pre-1.0 versioning: new features bump the minor version and fixes bump the patch
version.

Each entry corresponds to one push to `main` and one Git tag.

## [Unreleased]

### Added

- Water-level-controlled Valve 1 operation using the YwRobot/YUROBOT sensor on
  ADC3/PF4.
- Automatic Valve 1 to Valve 2 sequencing after the water-level threshold is
  reached.

### Changed

- Valve 1 now remains open until the filtered water signal reaches 100%.
- Valve 1 closes for the existing 5-second lockout, then Valve 2 opens for 10
  seconds and observes its own 5-second lockout.
- Valve close requests are accepted only for an active/open valve.

### Verified

- Firmware builds successfully; FLASH 27072 B, RAM 5064 B.
- Host tests pass 3/3.

### Pending hardware verification

- Confirm Valve 1 remains open until the sensor reports 100%.
- Confirm Valve 2 starts automatically after Valve 1's lockout.
- Confirm both closing lockouts and LD1 behavior on the board.

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
