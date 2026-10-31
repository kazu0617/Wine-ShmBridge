# Steam Frame / Resonite investigation archive

These files preserve local work from 2026-09-26, archived on 2026-10-31 before
cleaning the temporary Codex work directory. Existing upstream sources are kept
unchanged. Fork base: `1dc8cc7dcf962b392a572167060d6de65ff0f26b`.

## Saved sources

- `steam-frame-unixlib/`: x64 PE frontend, native Unixlib, shared ABI, builtin
  marker utility, export definitions, and the bridge round-trip test source.
- `steam-frame-tools/`: .NET bundle inspection and byte patch tools, ARM64 apphost
  binding, modified Linux bootstrap, early bridge experiments and debug helpers.
- `steam-frame-source-manifest.json`: original relative paths and SHA-256 hashes
  of every copied source file. No binaries, runtime packages, decompiled sources,
  game files, credentials, Steam configuration or raw logs are included.

## Historical outcome and limits

The original report records a successful PE -> Wine Unix call -> ARM64 POSIX
semaphore round trip and stable native host / Wine renderer IPC startup. The
renderer still produced black frames: usable Resonite UI / world rendering was
not verified. This archival pass does not claim fresh Steam Frame runtime tests.

The earlier unconditional Windows-semaphore fallback is **not a general fix**:
native Linux host and Wine renderer semaphore namespaces can differ. That source
change is preserved separately in `kazu0617/interprocess`, branch
`experiment/steam-frame-wine-semaphores`. The experimental Wine build guard
change is in `kazu0617/wine`, branch `experiment/arm64-non-pe-winebuild`.

## Reusing the tools

Most patch scripts expect to run from an investigation root containing `work/`.
Their original DLL / EXE inputs must be supplied locally from the matching build.
They are not shipped here. Keep those inputs and patched outputs out of Git.
Several scripts check exact bytes or offsets and are specific to those historical
assemblies; a version mismatch must be investigated, not patched blindly.

The Unixlib source must be compiled for the native architecture of the Wine host
(ARM64 for the investigated device); the PE frontend and test use an x64 Windows
toolchain. The Unix call ABI and builtin marking are Wine-version-dependent.
The original full cross-build command line was not recorded here, so this archive
is source preservation rather than a verified one-command cross-build recipe.

During archival, native C and MinGW C syntax checks and Python / shell syntax
checks are run. These do not establish ABI correctness or runtime compatibility.
