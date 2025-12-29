# PHP Printer Extension

A PHP extension for direct printing to Windows printers, providing low-level access to Windows GDI and Print Spooler APIs.

## Authors and Credits

**Original Authors:**
- Frank M. Kromann <frank@kromann.info>
- Daniel Beulshausen <daniel@php4win.de>

**Contributors:**
- Philippe MAES <luckyluke@dlfp.org>

**PHP 7.4+ Migration:**
- Updated from PHP 5.6 API to PHP 7.4+ API (2025)
- Compatible with PHP 7.4, 8.0, 8.1, 8.2, and 8.3

## Overview

This extension allows PHP scripts on Windows to:
- Open connections to printers
- Send raw data to printers
- Enumerate available printers
- Configure printer settings
- Draw graphics using GDI (pens, brushes, fonts)
- Print bitmaps
- Control document and page layout

## Platform Requirements

**Windows Only** - This extension uses Windows-specific APIs (GDI, winspool) and will only work on Windows systems.

## Prerequisites

### Windows Build Requirements
- Windows 10 or later
- PHP 7.4+ development files (PHP SDK) - supports PHP 7.4, 8.0, 8.1, 8.2, 8.3
- Visual Studio 2017 or later (or Visual Studio Build Tools)
- Windows SDK
- Git for Windows

### Linux
**Note:** This extension cannot be built or used on Linux as it depends on Windows-specific APIs. The source code can be reviewed on Linux, but compilation and usage require Windows.

## Building on Windows

This extension is designed to be built as an external (shared) extension, resulting in a `php_printer.dll` file that can be dynamically loaded by PHP.

### Method 1: Build as Standalone Extension (Recommended)

This method builds the extension separately from PHP, which is the recommended approach for most users.

#### Step 1: Prerequisites

- Install PHP 7.4+ or PHP 8.x on your Windows system
- Download and set up the PHP SDK for Windows:

```cmd
git clone https://github.com/php/php-sdk-binary-tools.git c:\php-sdk
cd c:\php-sdk
phpsdk-vs16-x64.bat
```

#### Step 2: Get Extension Source

```cmd
git clone https://github.com/apss-pohl/php_printer.git
cd php_printer
```

#### Step 3: Configure for Shared Build

```cmd
phpize
configure --enable-printer=shared --with-php-build=C:\php-sdk\phpdev\vs16\x64\deps
```

For PHP 7.4, use `vc15` instead of `vs16`:
```cmd
configure --enable-printer=shared --with-php-build=C:\php-sdk\phpdev\vc15\x64\deps
```

#### Step 4: Build

```cmd
nmake
```

#### Step 5: Install

Copy the compiled DLL to your PHP extensions directory:

```cmd
copy x64\Release_TS\php_printer.dll C:\php\ext\
```

Or for NTS (Non-Thread-Safe) builds:
```cmd
copy x64\Release\php_printer.dll C:\php\ext\
```

#### Step 6: Enable Extension

Add to your `php.ini`:
```ini
extension=php_printer
```

Verify installation:
```cmd
php -m | findstr printer
```

### Method 2: Build with PHP Source (Advanced)

This method builds the extension alongside PHP source code, still producing a shared extension.

#### Step 1: Get PHP Source

For PHP 7.4:
```cmd
phpsdk_buildtree phpdev
cd phpdev\vc15\x64
git clone https://github.com/php/php-src.git php-7.4
cd php-7.4
git checkout PHP-7.4
```

For PHP 8.x (e.g., PHP 8.3):
```cmd
phpsdk_buildtree phpdev
cd phpdev\vs16\x64
git clone https://github.com/php/php-src.git php-8.3
cd php-8.3
git checkout PHP-8.3
```

#### Step 2: Get Extension Source

```cmd
cd ext
git clone https://github.com/apss-pohl/php_printer.git printer
cd ..
```

#### Step 3: Configure and Build as Shared Extension

```cmd
buildconf
configure --disable-all --enable-cli --enable-printer=shared
nmake
```

#### Step 4: Install

Copy the compiled DLL:

```cmd
copy x64\Release_TS\php_printer.dll C:\php\ext\
```

Add to your `php.ini`:
```ini
extension=php_printer
```

Verify installation:
```cmd
php -m | findstr printer
```

## Quick Start Example

```php
<?php
// Check if extension is loaded
if (!extension_loaded('printer')) {
    die("Printer extension not loaded\n");
}

// List available printers
$printers = printer_list(PRINTER_ENUM_LOCAL);
foreach ($printers as $printer) {
    echo "Printer: {$printer['NAME']}\n";
}

// Open default printer
$handle = printer_open();
if (!$handle) {
    die("Failed to open printer\n");
}

// Write text
printer_write($handle, "Hello from PHP!");

// Close printer
printer_close($handle);
```

## Advanced Example - Drawing Graphics

```php
<?php
$handle = printer_open();

// Create device context
printer_create_dc($handle);

// Start document
printer_start_doc($handle, "My Document");
printer_start_page($handle);

// Create pen and font
$pen = printer_create_pen(PRINTER_PEN_SOLID, 2, "000000");
$font = printer_create_font("Arial", 24, 12, PRINTER_FW_NORMAL, false, false, false, 0);

printer_select_pen($handle, $pen);
printer_select_font($handle, $font);

// Draw text
printer_draw_text($handle, "Hello World!", 100, 100);

// Draw rectangle
printer_draw_rectangle($handle, 50, 50, 500, 300);

// End document
printer_end_page($handle);
printer_end_doc($handle);

// Cleanup
printer_delete_font($font);
printer_delete_pen($pen);
printer_delete_dc($handle);
printer_close($handle);
```

## API Functions

### Printer Management
- `printer_open([string $printername])` - Open printer connection
- `printer_close(resource $handle)` - Close printer connection
- `printer_write(resource $handle, string $data)` - Write raw data
- `printer_list(int $enumtype [, string $name [, int $level]])` - List printers
- `printer_abort(resource $handle)` - Abort current print job

### Configuration
- `printer_set_option(resource $handle, int $option, mixed $value)` - Set printer option
- `printer_get_option(resource $handle, int $option)` - Get printer option

### Device Context
- `printer_create_dc(resource $handle)` - Create device context
- `printer_delete_dc(resource $handle)` - Delete device context

### Document Control
- `printer_start_doc(resource $handle [, string $document])` - Start document
- `printer_end_doc(resource $handle)` - End document
- `printer_start_page(resource $handle)` - Start page
- `printer_end_page(resource $handle)` - End page

### Drawing Tools
- `printer_create_pen(int $style, int $width, string $color)` - Create pen
- `printer_delete_pen(resource $pen)` - Delete pen
- `printer_select_pen(resource $handle, resource $pen)` - Select pen
- `printer_create_brush(int $style, string $color_or_file)` - Create brush
- `printer_delete_brush(resource $brush)` - Delete brush
- `printer_select_brush(resource $handle, resource $brush)` - Select brush
- `printer_create_font(string $face, int $height, int $width, int $weight, bool $italic, bool $underline, bool $strikeout, int $orientation)` - Create font
- `printer_delete_font(resource $font)` - Delete font
- `printer_select_font(resource $handle, resource $font)` - Select font
- `printer_logical_fontheight(resource $handle, int $height)` - Get logical font height

### Drawing Functions
- `printer_draw_text(resource $handle, string $text, int $x, int $y)` - Draw text
- `printer_draw_line(resource $handle, int $fx, int $fy, int $tx, int $ty)` - Draw line
- `printer_draw_rectangle(resource $handle, int $ul_x, int $ul_y, int $lr_x, int $lr_y)` - Draw rectangle
- `printer_draw_roundrect(resource $handle, int $ul_x, int $ul_y, int $lr_x, int $lr_y, int $width, int $height)` - Draw rounded rectangle
- `printer_draw_elipse(resource $handle, int $ul_x, int $ul_y, int $lr_x, int $lr_y)` - Draw ellipse
- `printer_draw_chord(resource $handle, int $rec_x, int $rec_y, int $rec_x1, int $rec_y1, int $rad_x, int $rad_y, int $rad_x1, int $rad_y1)` - Draw chord
- `printer_draw_pie(resource $handle, int $rec_x, int $rec_y, int $rec_x1, int $rec_y1, int $rad1_x, int $rad1_y, int $rad2_x, int $rad2_y)` - Draw pie
- `printer_draw_bmp(resource $handle, string $filename, int $x, int $y [, int $width, int $height])` - Draw bitmap

## Constants

### Printer Enumeration
- `PRINTER_ENUM_LOCAL` - Local printers
- `PRINTER_ENUM_NAME` - Named printer
- `PRINTER_ENUM_SHARED` - Shared printers
- `PRINTER_ENUM_DEFAULT` - Default printer
- `PRINTER_ENUM_CONNECTIONS` - Printer connections
- `PRINTER_ENUM_NETWORK` - Network printers
- `PRINTER_ENUM_REMOTE` - Remote printers

### Pen Styles
- `PRINTER_PEN_SOLID` - Solid pen
- `PRINTER_PEN_DASH` - Dashed pen
- `PRINTER_PEN_DOT` - Dotted pen
- `PRINTER_PEN_DASHDOT` - Dash-dot pen
- `PRINTER_PEN_DASHDOTDOT` - Dash-dot-dot pen
- `PRINTER_PEN_INVISIBLE` - Invisible pen

### Brush Styles
- `PRINTER_BRUSH_SOLID` - Solid brush
- `PRINTER_BRUSH_CUSTOM` - Custom pattern brush
- `PRINTER_BRUSH_DIAGONAL` - Diagonal hatch
- `PRINTER_BRUSH_CROSS` - Cross hatch
- `PRINTER_BRUSH_DIAGCROSS` - Diagonal cross hatch
- `PRINTER_BRUSH_FDIAGONAL` - Forward diagonal
- `PRINTER_BRUSH_HORIZONTAL` - Horizontal hatch
- `PRINTER_BRUSH_VERTICAL` - Vertical hatch

### Font Weights
- `PRINTER_FW_THIN` - Thin
- `PRINTER_FW_ULTRALIGHT` - Ultra light
- `PRINTER_FW_LIGHT` - Light
- `PRINTER_FW_NORMAL` - Normal
- `PRINTER_FW_MEDIUM` - Medium
- `PRINTER_FW_BOLD` - Bold
- `PRINTER_FW_ULTRABOLD` - Ultra bold
- `PRINTER_FW_HEAVY` - Heavy

### Text Alignment
- `PRINTER_TA_BASELINE` - Baseline alignment
- `PRINTER_TA_BOTTOM` - Bottom alignment
- `PRINTER_TA_TOP` - Top alignment
- `PRINTER_TA_CENTER` - Center alignment
- `PRINTER_TA_LEFT` - Left alignment
- `PRINTER_TA_RIGHT` - Right alignment

### Paper Formats
- `PRINTER_FORMAT_CUSTOM` - Custom format
- `PRINTER_FORMAT_LETTER` - Letter (8.5 x 11 in)
- `PRINTER_FORMAT_LEGAL` - Legal (8.5 x 14 in)
- `PRINTER_FORMAT_A3` - A3 (297 x 420 mm)
- `PRINTER_FORMAT_A4` - A4 (210 x 297 mm)
- `PRINTER_FORMAT_A5` - A5 (148 x 210 mm)
- `PRINTER_FORMAT_B4` - B4 (250 x 354 mm)
- `PRINTER_FORMAT_B5` - B5 (182 x 257 mm)
- `PRINTER_FORMAT_FOLIO` - Folio (8.5 x 13 in)

### Orientation
- `PRINTER_ORIENTATION_PORTRAIT` - Portrait orientation
- `PRINTER_ORIENTATION_LANDSCAPE` - Landscape orientation

## Troubleshooting

### Extension not loading
- Verify `php_printer.dll` is in the extensions directory
- Check `extension=php_printer.dll` is in `php.ini`
- Ensure you're using PHP 7.4+ (check with `php -v`)
- Check for missing dependencies with Dependency Walker

### Printer not found
- Verify printer is installed and online in Windows
- Check printer name matches exactly (case-sensitive)
- Try using `printer_list()` to see available printers

### Build errors
- Ensure Visual Studio 2017 or later is installed
- Verify PHP SDK is properly configured
- Check that Windows SDK is installed
- Make sure you're in the PHP SDK environment (`phpsdk-vs16-x64.bat`)

## Version History

- **v0.1.0-dev** - Initial PHP 4/5 version
- **v0.2.0** - PHP 7.4+ compatibility update (2025)
  - Migrated from PHP 5.6 API to PHP 7.4+ API
  - Updated all deprecated functions
  - Modern parameter parsing
  - Updated resource management
  - Improved error handling
  - Compatible with PHP 7.4, 8.0, 8.1, 8.2, and 8.3

## License

This source file is subject to version 2.02 of the PHP license. See [LICENSE](http://www.php.net/license/2_02.txt) for details.

Copyright (c) 1997-2003 The PHP Group

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on Windows with PHP 7.4+ or PHP 8.x
5. Submit a pull request

## Support

For issues and questions:
- Open an issue on GitHub
- Check existing documentation
- Review example code

## See Also

- [PHP Manual](https://www.php.net/manual/)
- [Windows GDI Documentation](https://docs.microsoft.com/en-us/windows/win32/gdi/)
- [Windows Print Spooler API](https://docs.microsoft.com/en-us/windows/win32/printdocs/)
