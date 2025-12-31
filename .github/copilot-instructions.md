# Copilot Coding Agent Instructions for `apss-pohl/php_printer`

These instructions are for an automated coding agent seeing this repository for the first time. They are **about understanding and maintaining the source code**, not about building or installing the extension. **Do not spend time trying to compile, package, or install** unless a task explicitly demands that and the user provides the necessary environment information.

If something here appears incomplete or inconsistent with the current tree, you may perform targeted searches, but treat this file as your primary map of the project's structure and design.

---

## 1. High-level Purpose and Scope

- This repository implements **PHP Printer Extension**, a **PHP extension written in C** that provides printing functionality to PHP.
- The extension allows developers to:
  - Open connections to printers
  - Send raw data to printers
  - Enumerate available printers
  - Configure basic printer settings
  - **Windows Only:** Draw graphics using GDI (pens, brushes, fonts)
  - **Windows Only:** Print bitmaps
  - **Windows Only:** Advanced document and page layout control
- Major external libraries wrapped here:
  - **Windows:** Windows GDI and Print Spooler APIs
  - **Linux:** CUPS (Common Unix Printing System)

**Target platforms:**

- **Linux and Windows only.** macOS is not officially supported.

**API reference:**

- The implementation uses:
  - **Windows:** Win32 GDI and Print Spooler APIs
  - **Linux:** CUPS library API (libcups)
  
When in doubt about behavior or signatures, consult the official Windows GDI documentation or CUPS documentation.

**PHP compatibility goal:**  
Code should remain compatible with **PHP 7.4 and all newer PHP versions**. When you make changes:

- Avoid relying on PHP API features that are version-specific unless guarded appropriately.
- Keep the extension's behavior consistent across PHP 7.4 and newer PHP 8.x releases.

---

## 2. Repository Layout and Architectural Overview

### 2.1. Top-level structure

Important files and directories in the repo root:

- `README.md` – Overview of the PHP Printer Extension, usage examples, build instructions.
- `CREDITS.txt` – Project authors and contributors.
- `printer.c` – Main C implementation of the extension.
- `php_printer.h` – Header file with function declarations and data structures.
- `config.m4` – Build configuration for Linux/Unix (autoconf).
- `config.w32` – Build configuration for Windows.
- `printer.php` – PHP stubs/constants for the extension.
- `example_cross_platform.php` – Example PHP script demonstrating printer usage.
- `printer.dsp` – Visual Studio project file (legacy).
- `.github/` – GitHub Actions workflows and documentation.

### 2.2. Core architecture and responsibilities

At a high level:

1. **Platform-specific implementations**:

   - The extension uses conditional compilation (`#ifdef PHP_WIN32`, `#ifdef HAVE_CUPS`) to support different platforms.
   - **Windows path:** Uses Windows GDI and Print Spooler APIs for full-featured printing including graphics.
   - **Linux path:** Uses CUPS for basic printing functionality.

2. **Resource management**:

   - The extension defines several resource types using `le_printer`, `le_brush`, `le_pen`, `le_font`.
   - These resources are registered with the PHP engine and properly cleaned up.

3. **PHP API integration**:

   - Uses standard PHP extension macros and functions.
   - Functions are registered via `PHP_FE()` macros.
   - Module initialization (`PHP_MINIT_FUNCTION`), request init/shutdown, and module info (`PHP_MINFO_FUNCTION`) are implemented.

---

## 3. Working with the Source Code

### 3.1. Adding or modifying printing functionality

When implementing new functionality or fixing bugs:

1. **Determine the target platform**:

   - Windows-specific features (GDI graphics) → Implement within `#ifdef PHP_WIN32` blocks.
   - Linux-specific features (CUPS) → Implement within `#ifdef HAVE_CUPS` blocks.
   - Cross-platform features → Provide implementations for both platforms.

2. **Use appropriate APIs as reference**:

   - For Windows: Consult MSDN documentation for GDI and Print Spooler functions.
   - For Linux: Consult CUPS API documentation.
   - Ensure that:
     - Function signatures match the underlying C APIs.
     - Parameter types and semantics are correct.
     - Memory management (allocation/deallocation) is handled properly.

3. **Locate existing, similar functions as a reference**:

   - Look for comparable functions in `printer.c`.
   - Match patterns such as:
     - Function naming conventions.
     - Parameter parsing using `zend_parse_parameters()`.
     - Resource fetching using `ZEND_FETCH_RESOURCE()`.
     - Return value handling.

4. **Maintain PHP extension conventions**:

   - Use standard PHP extension macros (`PHP_FUNCTION`, `ZEND_BEGIN_ARG_INFO`, etc.).
   - Ensure proper error handling and return values.
   - Follow PHP coding standards for C extensions.
   - Handle memory correctly:
     - Use `emalloc()`/`efree()` for request-scoped memory.
     - Use `pemalloc()`/`pefree()` for persistent memory.
     - Properly manage reference counts for resources.

5. **Compatibility with PHP 7.4 and newer**:
   - Use APIs supported across PHP 7.4 and PHP 8.x versions.
   - Be aware of API changes between PHP 7 and PHP 8.
   - Test code paths for both major versions when possible.

### 3.2. Cross-cutting concerns

When modifying cross-platform areas:

- Ensure both Windows and Linux code paths are updated consistently.
- Keep platform-specific code clearly separated with preprocessor directives.
- Document any platform-specific limitations or behaviors.
- Test changes on both platforms when possible, or clearly note if testing was limited to one platform.

---

## 4. External Reference for Semantics

Instead of relying solely on in-repo documentation, **use official platform documentation as your reference**:

- **Windows GDI:**
  - Microsoft Docs: Windows GDI and Print Spooler APIs
  - https://docs.microsoft.com/en-us/windows/win32/gdi/
  - https://docs.microsoft.com/en-us/windows/win32/printdocs/

- **CUPS (Linux):**
  - CUPS API documentation: https://www.cups.org/doc/api-cups.html
  - CUPS PPD API: https://www.cups.org/doc/api-ppd.html

When you need to reason about how a function _should_ behave:

1. Identify the underlying platform API function.
2. Look up its definition and documentation in the official platform documentation.
3. Mirror the behavior in the PHP extension wrapper, ensuring proper error handling and type conversions.

---

## 5. How to Approach Tasks in this Repo

When receiving a task related to this repository:

1. **Assume a working build/CI environment exists outside your control.**  
   Do **not** spend time trying to:

   - Discover or modify compiler flags.
   - Adjust or create new build scripts or Makefile targets.
   - Write instructions on how to install the extension.

2. **Focus on source-level changes**:

   - Implement or adjust C code in `printer.c`.
   - Update `php_printer.h` when adding new functions or data structures.
   - Ensure proper platform-specific handling with preprocessor directives.

3. **Preserve compatibility with PHP 7.4 and all newer PHP versions**:

   - Keep PHP-facing APIs stable and predictable.
   - Avoid introducing behaviors that rely on undefined or version-specific differences in PHP internals.
   - When in doubt, follow existing patterns in this codebase.

4. **Only search further when necessary:**
   - Start from the files listed here.
   - Use code search to find existing patterns to follow.
   - Expand your search scope only when this file and the directly-related code are insufficient.

By following this guidance, you will maximize your effectiveness on tasks involving this repository while minimizing time spent on non-essential build or environment details.
