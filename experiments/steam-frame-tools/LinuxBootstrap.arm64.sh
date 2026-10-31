#!/usr/bin/env sh

# This is a bootstrapper to run Resonite in what is quite the unorthodox
# configuration on Linux.
#
# This script is executed by the "Renderite.Boot" bootstrapper if it
# detects it's running under Proton/Wine.
#
# The following shell code is responsible for installing an adequate runtime
# for Resonite that's independent from any system installation (or lack thereof)
# and for ensuring that Renderite is executed with Proton's Wine installation
# and not the system's.


### SCRIPT START ###

# Always resolve paths relative to this script. Proton normally starts the
# bootstrapper in the game directory, but relying on that makes manual and
# alternate launch paths fragile.

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
# Wine commonly exposes the Steam library through a dosdevices path such as
# .../pfx/dosdevices/s:/steamapps/....  A colon is a path-list separator in
# PATH and LD_LIBRARY_PATH, so canonicalize the drive symlink before exporting
# either variable.
if command -v readlink >/dev/null 2>&1; then
	CANONICAL_SCRIPT_DIR=$(readlink -f -- "$SCRIPT_DIR" 2>/dev/null || true)
	if [ -n "$CANONICAL_SCRIPT_DIR" ]; then
		SCRIPT_DIR=$CANONICAL_SCRIPT_DIR
	fi
fi
cd "$SCRIPT_DIR" || exit 1

# Ensure that $DOTNET_ROOT points to the downloaded runtime rather
# than at the system's. Also ensure that the root and tools subfolder
# are temporarily added to the system's $PATH variable just to make
# sure things work correctly.

DOTNET_ROOT="$PWD/dotnet-runtime"
PATH="$PATH":"$DOTNET_ROOT":"$DOTNET_ROOT"/tools
export DOTNET_ROOT PATH

# The Steam depot contains native assets for several Linux architectures.
# The files flattened into the application root are x86_64, so an aarch64
# host must put its RID-specific directory first or P/Invoke loads an x64 ELF
# and fails with BadImageFormatException.

case "$(uname -m)" in
	aarch64|arm64)
		DOTNET_ARCH=arm64
		NATIVE_RID=linux-arm64
		;;
	x86_64|amd64)
		DOTNET_ARCH=x64
		NATIVE_RID=linux-x64
		;;
	*)
		DOTNET_ARCH=
		NATIVE_RID=
		;;
esac

if [ -n "$NATIVE_RID" ] && [ -d "$PWD/runtimes/$NATIVE_RID/native" ]; then
	LD_LIBRARY_PATH="$PWD/runtimes/$NATIVE_RID/native${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
	export LD_LIBRARY_PATH
fi


# Miscellaneous variables

DOTNET_INSTALL_SCRIPT="$PWD/dotnet-install.sh"
DOTNET_EXECUTABLE="$DOTNET_ROOT/dotnet"

# terminal_execute()
# {
# 	# !! Modified by Cyro !! (Change: terminal emulator order)
# 	#
# 	# This code is released in public domain by Han Boetes <han@mijncomputer.nl>
# 	#
# 	# This script tries to exec a terminal emulator by trying some known terminal
# 	# emulators.
# 	#
# 	# We welcome patches that add distribution-specific mechanisms to find the
# 	# preferred terminal emulator. On Debian, there is the x-terminal-emulator
# 	# symlink for example.
# 	#
# 	# Invariants:
# 	# 1. $TERMINAL comes first.
# 	# 2. The most common terminal emulators from popular desktop environments are tried afterwards.
# 	# 4. More niche/less well-known terminal emulators are tried next.
# 	# 3. Distribution-specific mechanisms come last (since they're kind of clunky), e.g. x-terminal-emulator, uxterm, xterm, etc.

# 	for terminal in "$TERMINAL" konsole gnome-terminal mate-terminal xfce4-terminal guake terminix tilix lxterminal terminator termite terminology tilda hyper alacritty termit kitty Eterm rio roxterm st lilyterm wezterm qterminal x-terminal-emulator uxterm xterm aterm urxvt rxvt; do
# 		if command -v "$terminal" > /dev/null 2>&1; then
# 			"$terminal" -e "$* 2>/dev/null"
# 			break
# 		fi
# 	done

# 	# "$*" 2>/dev/null
# }

# Overload the 'dotnet' command so that it calls the version which was
# downloaded by the script rather than potentially call (or fail to call)
# the system's main dotnet runtime.

dotnet()
{
	"$DOTNET_EXECUTABLE" "$@"
}

main()
{
	# Make sure the dotnet installer is executable, grab the .NET 9.0 runtime and
	# just place it in the main Resonite folder.

	chmod +x "$DOTNET_INSTALL_SCRIPT"

	# Install .NET 10 into the current directory

	if [ -n "$DOTNET_ARCH" ]; then
		bash "$DOTNET_INSTALL_SCRIPT" --verbose --channel 10.0 --runtime dotnet --architecture "$DOTNET_ARCH" --install-dir "$DOTNET_ROOT"
	else
		bash "$DOTNET_INSTALL_SCRIPT" --verbose --channel 10.0 --runtime dotnet --install-dir "$DOTNET_ROOT"
	fi


	# Not technically required, but mark dotnet itself as executable just in case.

	chmod +x "$DOTNET_EXECUTABLE"


	# ~ Launch Resonite! :) ~

	dotnet Renderite.Host.dll "$@"
}

main "$@"
exit
