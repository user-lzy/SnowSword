# SnowSword

**Windows 10/11 x64 Kernel Analysis & Anti-Rootkit Toolkit**

**English** | [简体中文](README.md)

SnowSword is a Windows kernel analysis and Anti-Rootkit toolkit built around a **Ring0 kernel driver** and a **Ring3 GUI/CLI**. It is intended for Windows kernel development, security research, rootkit detection, reverse engineering and low-level system analysis.

The project covers hidden process/driver detection, kernel callbacks, SSDT / Shadow SSDT, IDT / GDT, inline hooks, Minifilter, WFP, NTFS / FAT32, process memory and other Windows Internals topics.

## Highlights

- Hidden process and hidden driver detection
- Kernel module and kernel thread enumeration
- SSDT / Shadow SSDT inspection
- IDT / GDT inspection
- Kernel callback enumeration
- Driver Object / IRP Dispatch / Attached Device inspection
- Inline hook detection
- Minifilter and legacy file-system filter inspection
- WFP filter / callout and NDIS-related inspection
- TCP / UDP connection enumeration and hidden connection detection
- NTFS parsing, FAT32 enumeration and raw sector I/O
- Process memory read/write and memory-region inspection
- Registry inspection and offline Hive analysis
- GUI, CLI, JSON output and Agent Pipe mode

## Architecture

```text
┌─────────────────────────────────────────┐
│ Ring3                                    │
│ VisualFreeBasic GUI / CLI / Agent       │
└───────────────────┬─────────────────────┘
                    │ DeviceIoControl / IOCTL
                    ▼
┌─────────────────────────────────────────┐
│ Ring0                                    │
│ Windows Kernel Driver (C / WDK)         │
└───────────────────┬─────────────────────┘
                    │ Kernel APIs / internals
                    ▼
┌─────────────────────────────────────────┐
│ Windows Kernel / Drivers / File System  │
└─────────────────────────────────────────┘
```

## Compatibility

| Windows version | Status |
|---|---|
| Windows 10 1607 ~ 22H2 | Supported |
| Windows 11 21H2 ~ 24H2 | Supported |
| Other Windows versions | Not sufficiently tested |

- Current target architecture: **x64**.
- Some modules rely on undocumented Windows kernel implementation details or runtime pattern scanning, so Windows updates may affect individual features.

## Build Environment

### Ring0 Driver

- Visual Studio 2022
- Windows Driver Kit (WDK)
- x64 target

Open `ring0/SnowSword.sln`, select an x64 Debug or Release configuration, and build `SnowSword.sys`.

### Ring3 GUI / CLI

- VisualFreeBasic 5.9.7
- WinFBX
- x64 target (`-gen gas64`)

Open `ring3/SnowSword.ffp` in VisualFreeBasic and build `SnowSword.exe` with the 64-bit configuration.

## CLI Examples

```text
SnowSword.exe --cli
SnowSword.exe -c "process list"
SnowSword.exe -c "thread list --pid <PID>"
SnowSword.exe --format JSON ...
SnowSword.exe --agent-pipe <name>
```

The CLI interface is still under active development, so available commands may change.

## Project Structure

```text
SnowSword/
├─ ring0/                 # Windows kernel driver (C / WDK)
│  ├─ include/
│  └─ sources/
├─ ring3/                 # GUI / CLI (VisualFreeBasic)
│  ├─ forms/
│  ├─ modules/
│  └─ images/
├─ README.md
├─ README_EN.md
└─ LICENSE
```

## Relevant Topics

SnowSword may be useful as a reference if you are working on:

- Windows Kernel Development
- Windows Internals
- Anti-Rootkit / Rootkit Detection
- Kernel Security / Kernel Integrity
- Windows Driver Development
- Reverse Engineering
- Malware Analysis
- Digital Forensics
- NTFS / FAT32 Internals
- Minifilter / WFP / NDIS
- Kernel Callback / SSDT / IDT / GDT

## Project Status

SnowSword is under active development and refactoring. Some functionality touches undocumented kernel behavior, so compatibility can vary between Windows builds.

When reporting a problem, please include the Windows version and full build number, SnowSword version or commit SHA, the affected module, relevant error codes/logs/WinDbg output, and minimal reproduction steps.

## License

SnowSword is released under the [MIT License](LICENSE).

## Disclaimer

SnowSword is intended for **Windows kernel development, security research, malware analysis and authorized system diagnostics**. Do not use it for unauthorized access or other unlawful activity. Kernel-level experimentation may cause system crashes or data loss; use an isolated test environment whenever appropriate.
