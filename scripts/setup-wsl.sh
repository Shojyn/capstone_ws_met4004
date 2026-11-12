#!/usr/bin/env bash

set -Eeuo pipefail

readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
readonly REPO_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd -P)"
readonly FIRMWARE_DIR="$REPO_ROOT/src/wcs_controller"
readonly HOST_TEST_BUILD_DIR="$FIRMWARE_DIR/build/setup-host-tests"
readonly TOOLCHAIN_VERSION="14.2.Rel1"
readonly TOOLCHAIN_GCC_VERSION="14.2.1"
readonly TOOLCHAIN_ROOT="$HOME/.local/opt/arm-gnu-toolchain-14.2.rel1"
readonly MIN_CMAKE_VERSION="3.22"

assume_yes=false
apt_packages=()

if [[ -t 1 ]]; then
  readonly GREEN=$'\033[0;32m'
  readonly YELLOW=$'\033[0;33m'
  readonly RED=$'\033[0;31m'
  readonly RESET=$'\033[0m'
else
  readonly GREEN=''
  readonly YELLOW=''
  readonly RED=''
  readonly RESET=''
fi

log() {
  printf '%s\n' "$*"
}

ok() {
  printf '%s[OK]%s %s\n' "$GREEN" "$RESET" "$*"
}

notice() {
  printf '%s[INFO]%s %s\n' "$YELLOW" "$RESET" "$*"
}

fail() {
  printf '%s[ERROR]%s %s\n' "$RED" "$RESET" "$*" >&2
  exit 1
}

on_error() {
  local status=$?
  printf '%s[ERROR]%s Setup failed at line %s (exit status %s).\n' \
    "$RED" "$RESET" "$1" "$status" >&2
  exit "$status"
}

trap 'on_error "$LINENO"' ERR

usage() {
  cat <<'EOF'
Usage: bash scripts/setup-wsl.sh [--yes]

Set up and verify the Ubuntu/WSL development environment. The script asks
before changing apt-managed packages unless --yes is supplied.
EOF
}

for argument in "$@"; do
  case "$argument" in
    --yes|-y)
      assume_yes=true
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      usage >&2
      fail "Unknown option: $argument"
      ;;
  esac
done

command_exists() {
  command -v "$1" >/dev/null 2>&1
}

version_at_least() {
  local required=$1
  local actual=$2
  printf '%s\n%s\n' "$required" "$actual" | sort -V -C
}

cmake_version() {
  cmake --version | command sed -n '1s/^cmake version //p'
}

toolchain_is_current() {
  local detected_version
  command_exists arm-none-eabi-gcc || return 1
  command_exists arm-none-eabi-objcopy || return 1
  detected_version="$(arm-none-eabi-gcc -dumpversion)"
  version_at_least "$TOOLCHAIN_GCC_VERSION" "$detected_version"
}

confirm_package_changes() {
  if ! sudo -n true 2>/dev/null && [[ ! -t 0 ]]; then
    fail "sudo needs a password, but this is not an interactive terminal. Run the script from a WSL terminal."
  fi

  if "$assume_yes"; then
    return
  fi

  printf '%sContinue with apt package installation/update? [y/N] %s' "$YELLOW" "$RESET"
  local answer
  read -r answer
  case "$answer" in
    y|Y|yes|YES)
      ;;
    *)
      fail "Setup cancelled before package changes."
      ;;
  esac
}

install_apt_dependencies() {
  if ((${#apt_packages[@]} == 0)); then
    ok "Required commands are already installed and meet the minimum versions."
    return
  fi

  notice "The following apt packages are required: ${apt_packages[*]}"
  confirm_package_changes
  sudo -v
  sudo apt-get update
  sudo DEBIAN_FRONTEND=noninteractive apt-get install -y "${apt_packages[@]}"
  ok "Required apt packages are installed and current."
}

add_apt_package() {
  local package=$1
  local existing
  for existing in "${apt_packages[@]}"; do
    [[ "$existing" == "$package" ]] && return
  done
  apt_packages+=("$package")
}

prepare_apt_packages() {
  local detected_cmake

  if ! command_exists git; then
    add_apt_package git
  fi

  if ! command_exists cmake; then
    add_apt_package cmake
  else
    detected_cmake="$(cmake_version)"
    if ! version_at_least "$MIN_CMAKE_VERSION" "$detected_cmake"; then
      notice "CMake $detected_cmake is below the required minimum $MIN_CMAKE_VERSION."
      add_apt_package cmake
    else
      ok "CMake $detected_cmake is already available and current for this project."
    fi
  fi

  if command_exists ninja; then
    ok "Ninja $(ninja --version) is already available."
  else
    add_apt_package ninja-build
  fi

  if ! command_exists curl; then
    add_apt_package curl
  fi
  if ! command_exists xz; then
    add_apt_package xz-utils
  fi
  if ! command_exists tar; then
    add_apt_package tar
  fi
  if ! command_exists cc || ! command_exists make; then
    add_apt_package build-essential
  fi

  if ! toolchain_is_current; then
    notice "Arm GNU Toolchain is missing or below the required version; curl and xz-utils may be needed for installation."
    if ! command_exists curl; then
      add_apt_package curl
    fi
    if ! command_exists xz; then
      add_apt_package xz-utils
    fi
  fi
}

install_arm_toolchain() {
  local host_arch
  local archive_arch
  local archive_name
  local archive_url
  local download_dir
  local archive_path
  local extracted_dir
  local toolchain_bin="$TOOLCHAIN_ROOT/bin"

  if [[ -x "$toolchain_bin/arm-none-eabi-gcc" ]]; then
    export PATH="$toolchain_bin:$PATH"
  fi

  if toolchain_is_current; then
    ok "Arm GNU Toolchain $TOOLCHAIN_VERSION or newer is already available."
    return
  fi

  host_arch="$(uname -m)"
  case "$host_arch" in
    x86_64)
      archive_arch="x86_64"
      ;;
    aarch64)
      archive_arch="aarch64"
      ;;
    *)
      fail "Unsupported WSL architecture: $host_arch"
      ;;
  esac

  archive_name="arm-gnu-toolchain-14.2.rel1-${archive_arch}-arm-none-eabi.tar.xz"
  archive_url="https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/${archive_name}"
  download_dir="$HOME/Downloads"
  archive_path="$download_dir/$archive_name"
  extracted_dir="arm-gnu-toolchain-14.2.rel1-${archive_arch}-arm-none-eabi"

  notice "Arm GNU Toolchain $TOOLCHAIN_VERSION is missing or outdated."
  notice "Installing it per-user under $HOME/.local/opt; no repository path is used."
  mkdir -p "$download_dir" "$HOME/.local/opt"
  curl --fail --location --retry 3 --output "$archive_path" "$archive_url"

  local extraction_dir
  extraction_dir="$(mktemp -d)"
  trap 'rm -rf "$extraction_dir"' RETURN
  tar -xf "$archive_path" -C "$extraction_dir"
  [[ -x "$extraction_dir/$extracted_dir/bin/arm-none-eabi-gcc" ]] || \
    fail "Downloaded Arm toolchain archive has an unexpected layout."

  rm -rf "$TOOLCHAIN_ROOT"
  mv "$extraction_dir/$extracted_dir" "$TOOLCHAIN_ROOT"
  trap - RETURN
  rm -rf "$extraction_dir"

  export PATH="$toolchain_bin:$PATH"
  ok "Arm GNU Toolchain $TOOLCHAIN_VERSION installed."
}

configure_toolchain_path() {
  local toolchain_bin="$TOOLCHAIN_ROOT/bin"
  local path_line='export PATH="$HOME/.local/opt/arm-gnu-toolchain-14.2.rel1/bin:$PATH"'
  local bashrc="$HOME/.bashrc"

  if [[ ! -x "$toolchain_bin/arm-none-eabi-gcc" ]]; then
    notice "An expected Arm toolchain is already available elsewhere on PATH; no local PATH entry is needed."
    return
  fi

  if [[ ":$PATH:" != *":$toolchain_bin:"* ]]; then
    export PATH="$toolchain_bin:$PATH"
  fi

  if [[ ! -f "$bashrc" ]] || ! command grep -Fqx "$path_line" "$bashrc"; then
    printf '\n# Arm GNU Toolchain for this project\n%s\n' "$path_line" >> "$bashrc"
    notice "Added the Arm toolchain to $bashrc."
    notice "Open a new WSL terminal or run: source ~/.bashrc"
  else
    ok "Arm toolchain PATH entry is already configured."
  fi
}

verify_tools() {
  local tool
  for tool in git cmake ninja curl tar arm-none-eabi-gcc arm-none-eabi-objcopy; do
    command_exists "$tool" || fail "Required command is unavailable: $tool"
  done

  local detected_cmake
  detected_cmake="$(cmake_version)"
  version_at_least "$MIN_CMAKE_VERSION" "$detected_cmake" || \
    fail "CMake $detected_cmake is too old; CMake $MIN_CMAKE_VERSION or newer is required."
  toolchain_is_current || \
    fail "Arm GNU Toolchain $TOOLCHAIN_GCC_VERSION or newer was not detected on PATH."

  ok "Git: $(git --version)"
  ok "CMake: $detected_cmake"
  ok "Ninja: $(ninja --version)"
  ok "Arm GCC: $(arm-none-eabi-gcc --version | command sed -n '1p')"
  ok "All required tools passed version checks."
}

build_and_test() {
  pushd "$FIRMWARE_DIR" >/dev/null

  notice "Configuring and building the Debug firmware."
  cmake --preset Debug
  cmake --build --preset Debug
  ok "Debug firmware build succeeded."

  notice "Building and running host tests."
  cmake -S tests -B "$HOST_TEST_BUILD_DIR" -G Ninja
  cmake --build "$HOST_TEST_BUILD_DIR"
  ctest --test-dir "$HOST_TEST_BUILD_DIR" --output-on-failure
  ok "Host tests passed."

  popd >/dev/null
}

log "WSL development environment setup"
log "Repository: $REPO_ROOT"
notice "This script supports Ubuntu/Debian running inside WSL."
notice "It does not install Windows-only STM32CubeMX, J-Link Commander, or STM32CubeProgrammer."
notice "A successful build and host-test run do not verify physical board behavior."

if [[ -z "${WSL_DISTRO_NAME:-}" && "$(uname -r)" != *[Mm]icrosoft* && "$(uname -r)" != *[Ww][Ss][Ll]* ]]; then
  fail "This script must be run inside WSL, not native Linux, PowerShell, or Command Prompt."
fi
command_exists apt-get || fail "apt-get is required. Use an Ubuntu/Debian WSL distribution."
command_exists sudo || fail "sudo is required to install WSL packages."

prepare_apt_packages
install_apt_dependencies
install_arm_toolchain
configure_toolchain_path
verify_tools
build_and_test

log ""
ok "WSL development environment is ready."
log "Firmware: $FIRMWARE_DIR/build/Debug/wcs_controller.elf"
log "Host tests: $HOST_TEST_BUILD_DIR"
notice "Windows flashing tools and STM32CubeMX must be installed separately when needed."
