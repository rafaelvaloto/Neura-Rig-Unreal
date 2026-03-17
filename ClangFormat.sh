#!/bin/bash

# http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

PROJECT_DIR="$(dirname "${BASH_SOURCE[0]}")"

# Colors for output
# shellcheck disable=SC2034
RED='\033[0;31m'
# shellcheck disable=SC2034
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
# shellcheck disable=SC2034
BLUE='\033[0;34m'
NC='\033[0m' # No Color

function win_setup() {
	local vswhere
	vswhere="$(cygpath -u "/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe")"

	if [ ! -f "${vswhere}" ]
	then
		printf "${YELLOW}Warning: %s not found${NC}\n" "${vswhere}"
		return
	fi

	local clang_format
	clang_format="$("${vswhere}" -products '*' -latest -find VC/Tools/Llvm/bin/clang-format.exe)"

	local clang_format_dir
	clang_format_dir="$(cygpath -u "$(dirname "${clang_format}")")"
	export PATH="${clang_format_dir}:${PATH}"
}

case "${OSTYPE}" in
	"darwin"*)
		function nproc {
			sysctl -n hw.logicalcpu
	  	}
		;;
	"msys")
		win_setup
		;;
esac

# Print clang-format version so it is visible in CI logs and such
clang-format --version

# Build find arguments array
FIND_ARGS=("${PROJECT_DIR}" \( -name "*.h" -o -name "*.cpp" \))


# Standard excludes
FIND_ARGS+=(-not -path "*/.git/*")

# Reset IFS to default to ensure array expansion works correctly for the find command
OLD_IFS="$IFS"
unset IFS

find "${FIND_ARGS[@]}" -print0 | xargs -0 -P "$(nproc)" -n 100 clang-format -i

IFS="$OLD_IFS"