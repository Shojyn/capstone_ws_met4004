# FYP-MET4004
This repository should contain all the relevant code required to run the Aquaculture Digestibility Sample Collection System.

## Firmware and Hardware

- Board: **NUCLEO-F746ZG**, with an STM32F746ZG MCU and built-in ST-LINK debugger.
- Firmware project: `src/wcs_controller/`.
- Configuration source: `src/wcs_controller/wcs_controller.ioc`.
- Generated HAL and CMSIS sources are included in `Drivers/`.
- Current baseline initializes the board peripherals and uses a 72 MHz system clock. The LED GPIO is configured, but the application loop does not yet blink it.

## Development Environment

Contributors need a compatible toolchain, not identical installation paths. The tested workflow is **VS Code connected to WSL**, with Linux-native build tools. Windows STM32CubeMX is used for code generation, and Windows STM32CubeProgrammer can be used for flashing.

| Component | Requirement / tested version | Purpose |
| --- | --- | --- |
| Git | Available in the build environment | Clone and manage the repository |
| CMake | Minimum 3.22; tested with 3.31.6 in WSL | Configure the build |
| Ninja | Tested with 1.12.1 in WSL | Execute build steps |
| Arm GNU Toolchain | Tested with **14.2.Rel1**, GCC 14.2.1, target **arm-none-eabi** | Compile and link STM32 firmware |
| VS Code + WSL extension | Recommended for Windows users | Edit and run commands inside Linux |
| VS Code C/C++ and CMake Tools extensions | Installed/enabled in **WSL** | Code navigation and build integration |

The VS Code extensions do **not** replace the CMake, Ninja, or compiler executables. WSL builds require Linux versions of these tools; installing them on Windows alone is insufficient.

### Set up WSL build tools

1. Contributors require an Ubuntu WSL distribution and the VS Code **WSL** extension on Windows.
2. Git and Ninja are installed through the WSL terminal:

   ```bash
   sudo apt update
   sudo apt install git ninja-build
   ```

3. CMake 3.22 or newer is required. Ubuntu 22.04 provides a compatible version through:

   ```bash
   sudo apt install cmake
   ```

   Contributors seeking an exact version match can install the official [CMake 3.31.6 Linux release](https://github.com/Kitware/CMake/releases/tag/v3.31.6). The tested Ninja release is available from [Ninja 1.12.1](https://github.com/ninja-build/ninja/releases/tag/v1.12.1).

4. The recommended compiler package is the **Linux-hosted Arm GNU Toolchain 14.2.Rel1 for arm-none-eabi**. The following official archive supports x86-64 WSL distributions (`uname -m` prints `x86_64`):

   [arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz](https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz)

   All commands in this section run in **Ubuntu/WSL Bash**, not PowerShell. Contributors first check the Linux host architecture:

   ```bash
   uname -m
   ```

   The commands below apply when the result is `x86_64`. An `aarch64` result requires the corresponding Linux AArch64-hosted package instead. The **host** describes the computer running the compiler; the **arm-none-eabi target** describes the embedded firmware it produces.

   The team installation location is `$HOME/.local/opt/`, inside the Linux filesystem. `$HOME` normally expands to `/home/<linux-username>`; it is not the Windows user directory. This per-user installation requires no `sudo` for extraction and stays outside the Git repository.

   Contributors download and extract the archive with explicit paths, so the starting directory does not matter:

   ```bash
   sudo apt install curl xz-utils
   mkdir -p "$HOME/Downloads" "$HOME/.local/opt"
   curl --fail --location --retry 3 \
     --output "$HOME/Downloads/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz" \
     "https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz"
   ```

   After the download completes successfully, extraction uses:

   ```bash
   tar -xf "$HOME/Downloads/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz" \
     -C "$HOME/.local/opt"
   ```

   The archive creates its own versioned folder. The expected compiler location is:

   ```text
   /home/<linux-username>/.local/opt/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc
   ```

   Before configuring `PATH`, contributors verify the extracted compiler directly:

   ```bash
   "$HOME/.local/opt/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc" --version
   ```

   This should report **Arm GNU Toolchain 14.2.Rel1** and **GCC 14.2.1**. If the file is missing, the download/extraction location needs correction before proceeding.

   Contributors open their Bash startup file using `code ~/.bashrc` (or a Linux text editor) and add the following line once at the end, then save the file. Running this line only in a terminal would configure that shell temporarily; saving it in `~/.bashrc` makes it available in future interactive Bash terminals, including tmux panes:

   ```bash
   export PATH="$HOME/.local/opt/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi/bin:$PATH"
   ```

   The shell configuration is then reloaded:

   ```bash
   source ~/.bashrc
   hash -r
   command -v arm-none-eabi-gcc
   readlink -f "$(command -v arm-none-eabi-gcc)"
   arm-none-eabi-gcc --version
   ```

   The resolved path should match the versioned compiler location above. A path under `~/.local/bin` is also valid if it is a symbolic link resolving to that location, as in the original tested setup. If another compiler appears, `type -a arm-none-eabi-gcc` lists available matches for diagnosing `PATH` ordering.

   Contributors then reload the VS Code WSL window and open a new integrated terminal. Repeating the path and version checks there confirms that VS Code terminals can access the same Linux compiler. Existing tmux shells need `source ~/.bashrc` as well; new panes load it automatically when using interactive Bash.

5. Contributors verify the tools from the WSL terminal:

   ```bash
   cmake --version
   ninja --version
   arm-none-eabi-gcc --version
   which cmake ninja arm-none-eabi-gcc
   ```

   The paths should point to Linux tools, rather than Windows `.exe` files. The Arm archive includes the embedded libraries and binary utilities required by this project. Arm-based hosts need a matching host package instead of the x86-64 archive.

### Open the repository in VS Code

Contributors can use an existing clone or clone and open the repository with:

```bash
git clone https://github.com/Shojyn/capstone_ws_met4004.git
cd capstone_ws_met4004
code .
```

VS Code should indicate a **WSL** connection, with the following extensions installed and enabled in that remote environment:

- **C/C++** (`ms-vscode.cpptools`)
- **CMake Tools** (`ms-vscode.cmake-tools`)

The **repository root** serves as the workspace. The shared `.vscode/settings.json` uses `${workspaceFolder}/src/wcs_controller`, so it works regardless of where the repository is cloned. After changing tool paths, contributors reload VS Code and open a new terminal. If CMake Tools cannot find CMake, contributors set its **CMake Path** in their personal WSL remote settings rather than committing a machine-specific path.

## Build the firmware

No board connection or CubeMX installation is required to compile the checked-in sources.

Contributors run the following commands in WSL, starting from the repository root:

```bash
cd src/wcs_controller
cmake --preset Debug
cmake --build --preset Debug
```

The presets select Ninja and the Arm GCC toolchain automatically. The output is:

```text
src/wcs_controller/build/Debug/wcs_controller.elf
```

A Release build uses the following commands from the same firmware directory:

```bash
cmake --preset Release
cmake --build --preset Release
```

The output is `build/Release/wcs_controller.elf`. Debug was successfully built with the tested WSL toolchain above; a successful build alone does not verify behavior on the board.

Native Windows builds are also possible with Windows CMake, Ninja, and Arm GNU tools on `PATH`. Different environments require separate build directories; a CMake build directory should not be reused between Windows and WSL, or between different clones/toolchains.

## Regenerate or flash firmware

- **Regeneration:** contributors open `wcs_controller.ioc` in Windows STM32CubeMX. This baseline was generated with **CubeMX 6.18.1** and **STM32Cube FW_F7 V1.17.4**. The toolchain selection remains **CMake**, with **Keep User Code when re-generating** enabled. Contributors review the Git diff after generation and place custom code in preserved `USER CODE` sections or separate application files.
- **Flashing:** the Nucleo board connects through its **ST-LINK USB connector (CN1)** using a data cable. Windows STM32CubeProgrammer programs the built `.elf` through ST-LINK/SWD. A project under `/mnt/c/...` is accessible at the corresponding `C:\...` path on Windows. Flashing and board behavior still need hardware verification.

## Git and commit-message conventions

Commit messages follow the existing format:

```text
type(scope): short description
```

The description concisely states the actual change, preferably starting with an action such as `add`, `fix`, or `update`. The scope identifies the affected component; existing scopes include `stm`, `config`, and `README`.

| Type | Use for |
| --- | --- |
| `feat` | New functionality or a new firmware component |
| `fix` | Bug fixes |
| `chore` | Configuration, maintenance, and README updates, matching the existing history |
| `refactor` | Code restructuring without an intended behavior change |
| `test` | Tests and test infrastructure |

Examples:

```text
feat(stm): autogenerate stm firmware
chore(config): update git ignore and fix working directory
chore(README): document WSL setup and firmware build steps
feat(stm): add LD1 blink test
fix(stm): correct valve timeout handling
```

- Contributors keep commits focused so changes are traceable. For example, generated firmware is committed separately from build/editor configuration changes.
- Version-controlled files include `.gitignore`, `.ioc`, generated sources and headers, drivers, startup assembly, linker scripts, and CMake files.
- Build outputs and personal settings remain outside Git. `.gitignore` excludes build directories, compiler outputs, and `CMakeUserPresets.json`.
- Contributors review `git status` and the staged diff (`git diff --cached`) before committing, stage only intended files, and perform a build check for firmware changes.

# Commit Check
Each contributor adds an X next to their name and commits the change.\
ELTON:X369 Git is Tough\
MAX:\
NAVEEN:X\
JENSEN:
