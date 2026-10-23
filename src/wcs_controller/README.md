# Flashing the NUCLEO-F746ZG

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
