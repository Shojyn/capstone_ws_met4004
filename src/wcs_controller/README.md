# Flashing the NUCLEO-F746ZG

## Interrupt-driven manual valve control

The latching switches control the three valve commands independently:

| Switch input | Relay output | Interrupt |
| --- | --- | --- |
| PE5 (`btn_valve_1`) | PE2 (`valve_1`) | EXTI9_5 |
| PE6 (`btn_valve_2`) | PE3 (`valve_2`) | EXTI9_5 |
| PF0 (`btn_valve_3`) | PE4 (`valve_3`) | EXTI0 |

Each switch connects its input to 3.3 V when ON; the configured pull-down makes an open switch read LOW. Each debounced **OFF → ON** transition toggles only its corresponding valve's commanded state. **ON → OFF does not change the valve**; it rearms the next toggle after debounce. Holding a switch ON does not repeatedly toggle the valve. Multiple valves may be commanded open simultaneously.

For example, valve 1 starts CLOSED. Switching btn1 ON commands OPEN; switching btn1 OFF leaves it OPEN; switching btn1 ON again commands CLOSED. Relay output HIGH maintains OPEN and LOW maintains CLOSE, independently of the current switch position.

Both rising and falling edges invoke the generated EXTI handlers. The application callback records the input level and tick timestamp, restarting a 30 ms debounce interval on every edge. The main loop services pending events after the interval and reads the settled level before changing the output. It does not continuously sample idle inputs, and no delays or relay writes occur inside the interrupt callback. Short critical sections protect the shared interrupt/main-loop state; unsigned tick subtraction handles timestamp rollover.

Outputs initialize LOW and all commanded states reset to CLOSED. Each switch must first be observed LOW for 30 ms before its rising transition can toggle a valve. A switch already ON at startup does not open anything; it must be turned OFF, allowed to debounce, then turned ON. The commands do not confirm physical valve position. LD1 continues blinking with the configured nonblocking interval (currently 50 ms).

The GPIO mapping and commanded states are owned by `Core/Src/valve_controller.c`, with the API declared in `Core/Inc/valve_controller.h`:

```c
Valve_Set(VALVE_1, VALVE_OPEN);
Valve_Set(VALVE_1, VALVE_CLOSED);
Valve_Toggle(VALVE_1);
ValveState command = Valve_GetCommand(VALVE_1);
```

The switch/debounce code calls this API rather than directly driving relay GPIOs. The API is used from main-loop context, not interrupts. Future automation can issue explicit commands through the same interface; manual/automatic authority selection is not implemented yet.

After flashing, hardware checks should cover independent toggles, unchanged outputs on release, sustained ON states without repeated toggles, simultaneous switches, contact bounce, and a switch already ON during reset. All outputs should initially be LOW. The toggle behavior requires verification on the board.

## Supported workflow

The tested board currently identifies as **SEGGER J-Link** over USB. The LED firmware was successfully programmed using Windows J-Link Commander V7.98h. SEGGER Embedded Studio is not required: editing and compilation remain in VS Code/WSL with CMake, Ninja, and Arm GNU tools.

The onboard debugger normally ships as ST-LINK, but its present USB identity determines the flashing tool. A teammate's board that identifies as ST-LINK requires STM32CubeProgrammer or another ST-LINK-compatible tool instead of this J-Link script.

## Prerequisites

- The WSL build tools described in the [root README](../../README.md) are installed and available on `PATH`.
- The Windows [SEGGER J-Link Software and Documentation Pack](https://www.segger.com/downloads/jlink/) is installed with its USB drivers.
- The board connects to Windows through its debugger USB connector **CN1**, using a USB data cable.
- J-Link Commander, GDB Server, and other debugger sessions are closed before the script starts so they do not hold the probe.

Windows owns the USB device in this workflow. USB forwarding to WSL is unnecessary because WSL launches the Windows `JLink.exe` executable. The `JLink CDC UART Port` is the serial interface; its COM port number is not used for programming.

## One-command build and flash

From the repository root in a WSL terminal:

```bash
bash scripts/flash-jlink.sh
```

The script configures and builds the Debug preset, creates a fresh HEX file, and invokes J-Link Commander with the following settings:

| Setting | Value |
| --- | --- |
| Target | `STM32F746ZG` |
| Interface | SWD |
| Interface speed | 4000 kHz |
| Sequence | Reset, halt, load firmware, reset, run, exit |

The command programs the board immediately after a successful build. Build or conversion failures stop execution before programming; Commander is configured to exit on errors and its exit status is propagated.

For Release firmware:

```bash
bash scripts/flash-jlink.sh --preset Release
```

For a build and command-file check without connecting to hardware:

```bash
bash scripts/flash-jlink.sh --dry-run
```

Dry-run still requires the Windows Commander installation, but does not execute it. Generated files stay in the ignored `src/wcs_controller/build/<preset>/` directory:

- `wcs_controller.elf`: linked firmware and debugging information.
- `wcs_controller.hex`: address-bearing programming image.
- `flash.jlink`: generated Commander commands with the translated Windows image path.

Repository paths are resolved relative to the script. An invocation using the script's absolute path also works from another directory. The tested repository is on a Windows-mounted drive under `/mnt/c`; other locations must also be readable by the Windows executable through the path produced by `wslpath -w`.

### Commander location and probe selection

The script uses `JLINK_EXE` when provided, then checks `PATH`, then common SEGGER installation folders under `/mnt/c/Program Files` and `/mnt/c/Program Files (x86)`. Version sorting selects the last candidate when several installations are found; an override selects a specific installation deterministically.

An override uses a **WSL path**, quoted to preserve spaces:

```bash
JLINK_EXE='/mnt/c/Program Files/SEGGER/JLink_V798h/JLink.exe' \
  bash scripts/flash-jlink.sh
```

When multiple probes are connected, a serial number identifies the intended probe:

```bash
JLINK_SERIAL=123456789 bash scripts/flash-jlink.sh
```

The example serial number is a placeholder; contributors use the number reported by their J-Link installation. Personal executable paths and probe serial numbers belong in local environment settings rather than committed project files.

## Manual fallback

Contributors build and convert the firmware in WSL:

```bash
cd src/wcs_controller
cmake --preset Debug
cmake --build --preset Debug
arm-none-eabi-objcopy -O ihex build/Debug/wcs_controller.elf build/Debug/wcs_controller.hex
wslpath -w "$PWD/build/Debug/wcs_controller.hex"
```

The final command prints the Windows file path. Contributors then open **J-Link Commander (`JLink.exe`)**, enter `connect`, and select `STM32F746ZG`, `SWD`, and `4000` kHz. At the `J-Link>` prompt, the following commands program the firmware, with the placeholder replaced by the printed path:

```text
r
h
loadfile "C:\path\to\wcs_controller.hex"
```

After successful programming, the following commands start execution and close Commander:

```text
r
g
exit
```

The LED test toggles **LD1 (green, PB0)** every 500 ms. The manual process has been verified on the board; dry-run verification of the script does not itself verify automated hardware programming.

## Troubleshooting

| Symptom | Check |
| --- | --- |
| `cmake` or compiler not found | The Linux tool installation and shell `PATH`; existing shells may need `source ~/.bashrc`. |
| `JLink.exe` not found | Windows SEGGER installation location; `JLINK_EXE` can override discovery. |
| Waiting for GDB connection | J-Link GDB Server was opened instead of Commander. The flashing script uses `JLink.exe`, not `JLinkGDBServer.exe`. |
| No ST-LINK detected, but Windows shows J-Link | ST-LINK-only software cannot use this probe in J-Link mode. Commander is the appropriate tool. |
| Probe busy or connection fails | Other Commander/GDB/debugger sessions, CN1 connection, data cable, board power, Windows drivers, and probe selection. |
| Firmware file not found | The HEX file must exist and its translated Windows path must be accessible. |
| Programming succeeds but LD1 stays off | The correct image, reset/run sequence, and startup initialization. A debugger can check whether execution reached the main loop or stopped in `Error_Handler()`. |

## Optional VS Code integration

The script can also be invoked from the VS Code WSL integrated terminal. A future VS Code task can call the same script to provide a menu action without duplicating the build/programming logic. Full breakpoint debugging is separate and uses Arm GDB with J-Link GDB Server, rather than requiring SEGGER's IDE.
