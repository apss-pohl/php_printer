# PHP 7.4+ and PHP 8 Compatibility Changes

This document describes the changes made to make the printer extension compatible with PHP 7.4+ and PHP 8.x.

## Summary of Changes

The extension has been updated from PHP 5.6 API to PHP 7.4+ API with full PHP 8 compatibility. Major changes include:

### 1. Function Entry Declarations
- Added argument info structures for all functions (required in PHP 7+)
- Changed from `NULL` to proper `arginfo_*` structures
- Changed array terminator from `{NULL, NULL, NULL}` to `PHP_FE_END`

### 2. Parameter Parsing
- Replaced `zend_get_parameters_ex()` with `zend_parse_parameters()`
- Removed manual type conversion calls (`convert_to_string_ex()`, `convert_to_long_ex()`)
- Changed from `zval **` to `zval *` for parameter handling

### 3. String Handling
- Updated `RETURN_STRING()` macro - removed duplicate parameter (no longer needs copy flag)
- Updated `add_assoc_string()` - removed duplicate parameter

### 4. INI Value Handling
- Changed INI handler to use `zend_string` instead of `char*`
- Updated to use `ZSTR_VAL()` and `ZSTR_LEN()` macros

### 5. Resource Management
- Changed resource destructors signature from `zend_rsrc_list_entry *` to `zend_resource *`
- Replaced `ZEND_REGISTER_RESOURCE()` with `RETURN_RES(zend_register_resource())`
- Replaced `ZEND_FETCH_RESOURCE()` with `zend_fetch_resource()`
- Replaced `zend_list_delete()` with `zend_list_close()`

### 6. Thread Safety
- Removed all `TSRMLS_DC` and `TSRMLS_CC` macros (thread-safety is now automatic in PHP 7+)

### 7. Error Handling
- Replaced `WRONG_PARAM_COUNT` with `RETURN_THROWS()` when parameter parsing fails
- Updated `php_error_docref()` calls to remove `TSRMLS_CC`

### 8. Array Handling
- Updated array manipulation to use stack allocation instead of heap allocation for zval structures
- Changed from pointer arrays to value arrays where appropriate

### 9. PHP 8 Compatibility
- Replaced `zend_bool` with `bool` (deprecated in PHP 8.0)
- All APIs are compatible with PHP 8.0, 8.1, 8.2, and 8.3

## Building the Extension

This is a Windows-only extension designed to be built as a shared/external extension (DLL).

### Windows with PHP SDK

Build as a shared extension:
```cmd
cd path\to\php_printer
phpize
configure --enable-printer=shared
nmake
```

The build produces `php_printer.dll` which should be copied to your PHP extensions directory and loaded via `php.ini`:
```ini
extension=php_printer
```

### Requirements
- PHP 7.4+ or PHP 8.x development files
- Windows SDK
- Visual Studio or compatible C compiler
- winspool.lib and gdi32.lib

## Testing

After building, load the extension:
```ini
extension=php_printer.dll
```

Verify it's loaded:
```php
<?php
if (extension_loaded('printer')) {
    echo "Printer extension loaded successfully\n";
} else {
    echo "Failed to load printer extension\n";
}
?>
```

## Known Issues

None currently known. The extension should compile and work with PHP 7.4+ and PHP 8.x.

## Notes

- This extension only works on Windows (uses Windows GDI and Print Spooler APIs)
- The extension is compatible with PHP 7.4, 8.0, 8.1, 8.2, and 8.3
- All deprecated APIs have been updated for modern PHP versions
