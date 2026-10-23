#!/usr/bin/env bash
# Build in WSL, then program through Windows J-Link Commander.
set -euo pipefail

usage() {
    printf '%s\n' \
        'Usage: bash scripts/flash-jlink.sh [--preset Debug|Release] [--dry-run]' \
        'Default: build Debug, program STM32F746ZG over SWD, reset and run.' \
        '--dry-run builds and prepares the command file without contacting hardware.' \
        'JLINK_EXE: optional WSL path to Windows JLink.exe.' \
        'JLINK_SERIAL: optional probe serial number when multiple probes are connected.'
}

fail() { printf 'Error: %s\n' "$*" >&2; exit 1; }
preset=Debug
dry_run=false
while (($#)); do
    case "$1" in
        --preset)
            (($# >= 2)) || fail '--preset requires Debug or Release'
            preset=$2
            shift 2
            ;;
        --dry-run) dry_run=true; shift ;;
        -h|--help) usage; exit 0 ;;
        *) usage >&2; fail "Unknown argument: $1" ;;
    esac
done
[[ "$preset" == Debug || "$preset" == Release ]] || fail 'Preset must be Debug or Release'

script_dir=$(dirname -- "$(realpath -- "${BASH_SOURCE[0]}")")
project_dir=$(realpath -- "$script_dir/../src/wcs_controller")
for tool in cmake ninja arm-none-eabi-gcc arm-none-eabi-objcopy wslpath; do
    command -v "$tool" >/dev/null || fail "$tool is missing from PATH; see README.md"
done

# Search common Windows installations; a local override handles custom locations.
jlink=${JLINK_EXE:-}
if [[ -z "$jlink" ]]; then
    jlink=$(command -v JLink.exe || true)
fi
if [[ -z "$jlink" ]]; then
    shopt -s nullglob
    candidates=(/mnt/c/Program\ Files/SEGGER/JLink*/JLink.exe
                /mnt/c/Program\ Files\ \(x86\)/SEGGER/JLink*/JLink.exe)
    shopt -u nullglob
    if ((${#candidates[@]})); then
        mapfile -t candidates < <(printf '%s\n' "${candidates[@]}" | sort -V)
        jlink=${candidates[-1]}
    fi
fi
[[ -n "$jlink" && -f "$jlink" ]] || fail 'Windows JLink.exe not found. Install SEGGER J-Link or set JLINK_EXE to its WSL path.'

build_dir="$project_dir/build/$preset"
printf 'Building %s firmware...\n' "$preset"
# A subshell keeps preset resolution relative to the firmware project.
(
    cd -- "$project_dir"
    cmake --preset "$preset"
    cmake --build --preset "$preset"
)
hex="$build_dir/wcs_controller.hex"
arm-none-eabi-objcopy -O ihex "$build_dir/wcs_controller.elf" "$hex"
hex_windows=$(wslpath -w "$hex")
command_file="$build_dir/flash.jlink"
command_windows=$(wslpath -w "$command_file")
printf 'r\nh\nloadfile "%s"\nr\ng\nexit\n' "$hex_windows" > "$command_file"

args=(-Device STM32F746ZG -If SWD -Speed 4000 -AutoConnect 1
      -ExitOnError 1 -NoGui 1 -CommandFile "$command_windows")
if [[ -n "${JLINK_SERIAL:-}" ]]; then
    [[ "$JLINK_SERIAL" =~ ^[0-9]+$ ]] || fail 'JLINK_SERIAL must contain only digits'
    args+=(-USB "$JLINK_SERIAL")
fi
printf 'Commander: %s\nFirmware: %s\nCommand file: %s\n' "$jlink" "$hex" "$command_file"
if "$dry_run"; then
    printf 'Dry run complete. No probe connection or flashing was attempted.\n'
    exit 0
fi
printf 'Programming STM32F746ZG, then resetting and running...\n'
if "$jlink" "${args[@]}"; then
    printf 'J-Link completed successfully. Check LD1 on the board.\n'
else
    result=$?
    printf 'J-Link failed (exit %s). Close other debugger applications and check the USB connection.\n' "$result" >&2
    exit "$result"
fi
