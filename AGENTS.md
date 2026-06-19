# Repository Guidelines

## Project Structure & Module Organization

```
snowsword/
├── ring0/                  # Kernel driver (C, WDK)
│   ├── include/            # Header files (*.h)
│   ├── sources/            # Source files (*.c)
│   └── SnowSword.sln       # Visual Studio solution
├── ring3/                  # User-mode GUI (VisualFreeBasic)
│   ├── forms/              # Form files (*.Frm)
│   ├── modules/            # Include modules (*.Inc)
│   └── images/             # Icons and assets
├── bin/                    # Pre-built binaries (sys + exe)
├── version.json            # Version info and update URLs
├── SyncAndCommit.ps1       # Sync source + binaries to Git
└── AGENTS.md
```

- **Ring0** (`ring0/`): Windows kernel driver written in C. Compiled with **Visual Studio + WDK** targeting x64.
- **Ring3** (`ring3/`): User-mode application written in **VisualFreeBasic 6.0**. Communicates with the driver via IOCTL.
- **Binaries** (`bin/`): Compiled outputs tracked for release convenience. Ring0 produces `SnowSword.sys`; Ring3 produces `SnowSword.exe`.

## Build, Test, and Development Commands

| Action | Command / Tool |
|---|---|
| Build kernel driver (ring0) | Open `ring0/SnowSword.sln` in Visual Studio with WDK installed, select **x64 Debug**, and build |
| Build GUI (ring3) | Open the project in **VisualFreeBasic 6.0** and compile |
| Sync sources to repo | Run `SyncAndCommit.ps1` (copies ring0/ring3 sources + compiled binaries into the repo, then commits) |

- The driver must be signed or run with **testsigning** enabled on the target machine.
- Debug kernel code via **WinDbg** attached to a VM or the local kernel.

## Coding Style & Naming Conventions

- **C (ring0)**: Indent with tabs (VS default). K&R-style braces. Header filenames match the subsystem (e.g., `Process.h` → `Process.c`). Include guard style: no strict convention—follow existing headers.
- **VisualFreeBasic (ring3)**: Form names use `Frm` prefix (e.g., `FrmMain.frm`). Module names use `Mod` prefix (e.g., `ModMemory.Inc`).
- **General**: Match the existing code style in the file you are editing. Do not reformat unrelated code.

## Testing Guidelines

- **No automated test suite** exists for this project.
- Manual testing is required:
  1. Build the driver and exe.
  2. Load the driver on a test Windows VM (`sc create` / `sc start` or OSRLoader).
  3. Launch `SnowSword.exe` and exercise the relevant feature (callback enumeration, memory editing, etc.).
  4. Verify with WinDbg for kernel-side correctness.
- Mention your testing setup and results in the PR description.

## Commit & Pull Request Guidelines

- Commit messages use a **prefix-based** convention with Chinese descriptions:
  - `feat:` — new feature
  - `fix:` — bug fix
  - `clean:` — cleanup/refactor
  - `chore:` — maintenance
  - `r0(...)` / `r3(...)` — ring-specific changes
- **PRs should**:
  - Clearly describe the change and which ring (0, 3, or both) is affected.
  - Note the Windows versions tested against (e.g., Win10 22H2, Win11 23H2).
  - Include screenshots of the GUI if UI is affected.

## Security & Safety

- This project operates at the Windows kernel level. A bug in the driver **can crash or corrupt the OS**. Always test on a VM or a non-production machine.
- Never test on your primary workstation without a VM.
- Do not submit code with hardcoded offsets or unvalidated pointers without review.
