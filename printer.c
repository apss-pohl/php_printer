/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2003 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Frank M. Kromann    <frank@kromann.info>                    |
   |          Daniel Beulshausen  <daniel@php4win.de>                     |
   | Contribution:                                                        |
   |		  Philippe MAES       <luckyluke@dlfp.org>                    |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ext/standard/info.h"
#include "ext/standard/php_math.h"
#include "ext/standard/php_string.h"
#include "php.h"
#include "php_ini.h"

#ifdef HAVE_PRINTER

static int le_printer, le_brush, le_pen, le_font;

#ifdef PHP_WIN32
#include <windows.h>
#include <wingdi.h>
#include <winspool.h>

#include "php_printer.h"
#else
/* Linux/Unix CUPS support */
#ifdef HAVE_CUPS
#include <cups/cups.h>
#include <cups/ppd.h>

/* Define printer enumeration constants for Linux/CUPS compatibility */
#define PRINTER_ENUM_DEFAULT    1
#define PRINTER_ENUM_LOCAL      2
#endif

#include "php_printer.h"
#endif

static void printer_close(zend_resource *resource);
static void object_close(zend_resource *resource);
char *get_default_printer(void);

#ifdef PHP_WIN32
COLORREF hex_to_rgb(char *hex);
char *rgb_to_hex(COLORREF rgb);
#endif

/* Argument info declarations for PHP 7+ */
ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_open, 0, 0, 0)
ZEND_ARG_INFO(0, printername)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_close, 0, 0, 1)
ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_write, 0, 0, 2)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, content)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_list, 0, 0, 1)
ZEND_ARG_INFO(0, enumtype)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, level)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_set_option, 0, 0, 3)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, option)
ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_get_option, 0, 0, 2)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_create_dc, 0, 0, 1)
ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_delete_dc, 0, 0, 1)
ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_start_doc, 0, 0, 1)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, document)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_end_doc, 0, 0, 1)
ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_start_page, 0, 0, 1)
ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_end_page, 0, 0, 1)
ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_create_pen, 0, 0, 3)
ZEND_ARG_INFO(0, style)
ZEND_ARG_INFO(0, width)
ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_delete_pen, 0, 0, 1)
ZEND_ARG_INFO(0, pen)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_select_pen, 0, 0, 2)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, pen)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_create_brush, 0, 0, 2)
ZEND_ARG_INFO(0, style)
ZEND_ARG_INFO(0, color_or_file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_delete_brush, 0, 0, 1)
ZEND_ARG_INFO(0, brush)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_select_brush, 0, 0, 2)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, brush)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_create_font, 0, 0, 8)
ZEND_ARG_INFO(0, face)
ZEND_ARG_INFO(0, height)
ZEND_ARG_INFO(0, width)
ZEND_ARG_INFO(0, font_weight)
ZEND_ARG_INFO(0, italic)
ZEND_ARG_INFO(0, underline)
ZEND_ARG_INFO(0, strikeout)
ZEND_ARG_INFO(0, orientation)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_delete_font, 0, 0, 1)
ZEND_ARG_INFO(0, font)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_select_font, 0, 0, 2)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, font)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_logical_fontheight, 0, 0, 2)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_draw_roundrect, 0, 0, 7)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, ul_x)
ZEND_ARG_INFO(0, ul_y)
ZEND_ARG_INFO(0, lr_x)
ZEND_ARG_INFO(0, lr_y)
ZEND_ARG_INFO(0, width)
ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_draw_rectangle, 0, 0, 5)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, ul_x)
ZEND_ARG_INFO(0, ul_y)
ZEND_ARG_INFO(0, lr_x)
ZEND_ARG_INFO(0, lr_y)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_draw_text, 0, 0, 4)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, text)
ZEND_ARG_INFO(0, x)
ZEND_ARG_INFO(0, y)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_draw_elipse, 0, 0, 5)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, ul_x)
ZEND_ARG_INFO(0, ul_y)
ZEND_ARG_INFO(0, lr_x)
ZEND_ARG_INFO(0, lr_y)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_draw_line, 0, 0, 5)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, fx)
ZEND_ARG_INFO(0, fy)
ZEND_ARG_INFO(0, tx)
ZEND_ARG_INFO(0, ty)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_draw_chord, 0, 0, 9)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, rec_x)
ZEND_ARG_INFO(0, rec_y)
ZEND_ARG_INFO(0, rec_x1)
ZEND_ARG_INFO(0, rec_y1)
ZEND_ARG_INFO(0, rad_x)
ZEND_ARG_INFO(0, rad_y)
ZEND_ARG_INFO(0, rad_x1)
ZEND_ARG_INFO(0, rad_y1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_draw_pie, 0, 0, 9)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, rec_x)
ZEND_ARG_INFO(0, rec_y)
ZEND_ARG_INFO(0, rec_x1)
ZEND_ARG_INFO(0, rec_y1)
ZEND_ARG_INFO(0, rad1_x)
ZEND_ARG_INFO(0, rad1_y)
ZEND_ARG_INFO(0, rad2_x)
ZEND_ARG_INFO(0, rad2_y)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_draw_bmp, 0, 0, 4)
ZEND_ARG_INFO(0, handle)
ZEND_ARG_INFO(0, filename)
ZEND_ARG_INFO(0, x)
ZEND_ARG_INFO(0, y)
ZEND_ARG_INFO(0, width)
ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printer_abort, 0, 0, 1)
ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO()

zend_function_entry printer_functions[] = {
    PHP_FE(printer_open, arginfo_printer_open) PHP_FE(
        printer_close,
        arginfo_printer_close) PHP_FE(printer_write,
                                      arginfo_printer_write) PHP_FE(printer_list,
                                                                    arginfo_printer_list)
        PHP_FE(printer_set_option, arginfo_printer_set_option) PHP_FE(
            printer_get_option,
            arginfo_printer_get_option) PHP_FE(printer_create_dc,
                                               arginfo_printer_create_dc)
            PHP_FE(printer_delete_dc, arginfo_printer_delete_dc) PHP_FE(
                printer_start_doc,
                arginfo_printer_start_doc) PHP_FE(printer_end_doc,
                                                  arginfo_printer_end_doc)
                PHP_FE(printer_start_page, arginfo_printer_start_page) PHP_FE(
                    printer_end_page,
                    arginfo_printer_end_page) PHP_FE(printer_create_pen,
                                                     arginfo_printer_create_pen)
                    PHP_FE(printer_delete_pen, arginfo_printer_delete_pen) PHP_FE(
                        printer_select_pen,
                        arginfo_printer_select_pen) PHP_FE(printer_create_brush,
                                                           arginfo_printer_create_brush)
                        PHP_FE(printer_delete_brush, arginfo_printer_delete_brush) PHP_FE(
                            printer_select_brush, arginfo_printer_select_brush)
                            PHP_FE(printer_create_font, arginfo_printer_create_font) PHP_FE(
                                printer_delete_font,
                                arginfo_printer_delete_font)
                                PHP_FE(printer_select_font, arginfo_printer_select_font) PHP_FE(
                                    printer_logical_fontheight,
                                    arginfo_printer_logical_fontheight)
                                    PHP_FE(printer_draw_roundrect,
                                           arginfo_printer_draw_roundrect)
                                        PHP_FE(printer_draw_rectangle,
                                               arginfo_printer_draw_rectangle)
                                            PHP_FE(printer_draw_text,
                                                   arginfo_printer_draw_text)
                                                PHP_FE(
                                                    printer_draw_elipse,
                                                    arginfo_printer_draw_elipse)
                                                    PHP_FE(
                                                        printer_draw_line,
                                                        arginfo_printer_draw_line)
                                                        PHP_FE(
                                                            printer_draw_chord,
                                                            arginfo_printer_draw_chord)
                                                            PHP_FE(
                                                                printer_draw_pie,
                                                                arginfo_printer_draw_pie)
                                                                PHP_FE(
                                                                    printer_draw_bmp,
                                                                    arginfo_printer_draw_bmp)
                                                                    PHP_FE(
                                                                        printer_abort,
                                                                        arginfo_printer_abort)
                                                                        PHP_FE_END};

zend_module_entry printer_module_entry = {STANDARD_MODULE_HEADER,
                                          "printer",
                                          printer_functions,
                                          PHP_MINIT(printer),
                                          PHP_MSHUTDOWN(printer),
                                          NULL,
                                          NULL,
                                          PHP_MINFO(printer),
                                          PHP_PRINTER_VERSION,
                                          STANDARD_MODULE_PROPERTIES};

ZEND_DECLARE_MODULE_GLOBALS(printer)

#ifdef COMPILE_DL_PRINTER
ZEND_GET_MODULE(printer)
#endif

PHP_MINFO_FUNCTION(printer) {
  php_info_print_table_start();
  php_info_print_table_header(2, "Printer Support", "enabled");
  php_info_print_table_row(2, "Version", PHP_PRINTER_VERSION);
#ifdef PHP_WIN32
  php_info_print_table_row(2, "Platform", "Windows (GDI)");
  php_info_print_table_row(2, "Default printing device",
                           PRINTERG(default_printer) ? PRINTERG(default_printer)
                                                     : "<b>not detected</b>");
#else
#ifdef HAVE_CUPS
  php_info_print_table_row(2, "Platform", "Linux/Unix (CUPS)");
  php_info_print_table_row(2, "Default printing device",
                           PRINTERG(default_printer) ? PRINTERG(default_printer)
                                                     : "<b>not detected</b>");
#else
  php_info_print_table_row(2, "Platform", "Unsupported (CUPS not available)");
#endif
#endif
  php_info_print_table_row(2, "Module state", "working");
  php_info_print_table_row(2, "RCS Version", "$Id$");
  php_info_print_table_end();
  DISPLAY_INI_ENTRIES();
}

static PHP_INI_MH(OnUpdatePrinter) {
  if (new_value != NULL && ZSTR_LEN(new_value) > 0) {
    if (PRINTERG(default_printer)) {
      pefree(PRINTERG(default_printer), 1);
    }
    PRINTERG(default_printer) = pestrdup(ZSTR_VAL(new_value), 1);
  }
  return SUCCESS;
}

PHP_INI_BEGIN()
PHP_INI_ENTRY("printer.default_printer", "", PHP_INI_ALL, OnUpdatePrinter)
PHP_INI_END()

#define COPIES 0
#define MODE 1
#define TITLE 2
#define ORIENTATION 3
#define YRESOLUTION 4
#define XRESOLUTION 5
#define PAPER_FORMAT 6
#define PAPER_LENGTH 7
#define PAPER_WIDTH 8
#define SCALE 9
#define BG_COLOR 10
#define TEXT_COLOR 11
#define TEXT_ALIGN 12
#define DEVICENAME 13
#define DRIVER_VERSION 14
#define OUTPUT_FILE 15
#define VALID_OPTIONS 16
#define BRUSH_SOLID (-1)
#define BRUSH_CUSTOM (-2)

#define REGP_CONSTANT(a, b)                                                    \
  REGISTER_LONG_CONSTANT(a, b, CONST_CS | CONST_PERSISTENT);

static void php_printer_init(zend_printer_globals *printer_globals) {
  printer_globals->default_printer = get_default_printer();
}

static void php_printer_shutdown(zend_printer_globals *printer_globals) {
  if (printer_globals->default_printer) {
    pefree(printer_globals->default_printer, 1);
  }
}

PHP_MINIT_FUNCTION(printer) {
  ZEND_INIT_MODULE_GLOBALS(printer, php_printer_init, php_printer_shutdown);
  REGISTER_INI_ENTRIES();

  le_printer = zend_register_list_destructors_ex(printer_close, NULL, "printer",
                                                 module_number);
  le_pen = zend_register_list_destructors_ex(object_close, NULL, "printer pen",
                                             module_number);
  le_font = zend_register_list_destructors_ex(object_close, NULL,
                                              "printer font", module_number);
  le_brush = zend_register_list_destructors_ex(object_close, NULL,
                                               "printer brush", module_number);

#ifdef PHP_WIN32
  /* Windows-specific constants */
  REGP_CONSTANT("PRINTER_COPIES", COPIES);
  REGP_CONSTANT("PRINTER_MODE", MODE);
  REGP_CONSTANT("PRINTER_TITLE", TITLE);
  REGP_CONSTANT("PRINTER_DEVICENAME", DEVICENAME);
  REGP_CONSTANT("PRINTER_DRIVERVERSION", DRIVER_VERSION);
  REGP_CONSTANT("PRINTER_OUTPUT_FILE", OUTPUT_FILE);
  REGP_CONSTANT("PRINTER_RESOLUTION_Y", YRESOLUTION);
  REGP_CONSTANT("PRINTER_RESOLUTION_X", XRESOLUTION);
  REGP_CONSTANT("PRINTER_SCALE", SCALE);
  REGP_CONSTANT("PRINTER_BACKGROUND_COLOR", BG_COLOR);
  REGP_CONSTANT("PRINTER_PAPER_LENGTH", PAPER_LENGTH);
  REGP_CONSTANT("PRINTER_PAPER_WIDTH", PAPER_WIDTH);

  REGP_CONSTANT("PRINTER_PAPER_FORMAT", PAPER_FORMAT);
  REGP_CONSTANT("PRINTER_FORMAT_CUSTOM", 0);
  REGP_CONSTANT("PRINTER_FORMAT_LETTER", DMPAPER_LETTER);
  REGP_CONSTANT("PRINTER_FORMAT_LEGAL", DMPAPER_LEGAL);
  REGP_CONSTANT("PRINTER_FORMAT_A3", DMPAPER_A3);
  REGP_CONSTANT("PRINTER_FORMAT_A4", DMPAPER_A4);
  REGP_CONSTANT("PRINTER_FORMAT_A5", DMPAPER_A5);
  REGP_CONSTANT("PRINTER_FORMAT_B4", DMPAPER_B4);
  REGP_CONSTANT("PRINTER_FORMAT_B5", DMPAPER_B5);
  REGP_CONSTANT("PRINTER_FORMAT_FOLIO", DMPAPER_FOLIO);

  REGP_CONSTANT("PRINTER_ORIENTATION", ORIENTATION);
  REGP_CONSTANT("PRINTER_ORIENTATION_PORTRAIT", DMORIENT_PORTRAIT);
  REGP_CONSTANT("PRINTER_ORIENTATION_LANDSCAPE", DMORIENT_LANDSCAPE);

  REGP_CONSTANT("PRINTER_TEXT_COLOR", TEXT_COLOR);
  REGP_CONSTANT("PRINTER_TEXT_ALIGN", TEXT_ALIGN);
  REGP_CONSTANT("PRINTER_TA_BASELINE", TA_BASELINE);
  REGP_CONSTANT("PRINTER_TA_BOTTOM", TA_BOTTOM);
  REGP_CONSTANT("PRINTER_TA_TOP", TA_TOP);
  REGP_CONSTANT("PRINTER_TA_CENTER", TA_CENTER);
  REGP_CONSTANT("PRINTER_TA_LEFT", TA_LEFT);
  REGP_CONSTANT("PRINTER_TA_RIGHT", TA_RIGHT);

  REGP_CONSTANT("PRINTER_PEN_SOLID", PS_SOLID);
  REGP_CONSTANT("PRINTER_PEN_DASH", PS_DASH);
  REGP_CONSTANT("PRINTER_PEN_DOT", PS_DOT);
  REGP_CONSTANT("PRINTER_PEN_DASHDOT", PS_DASHDOT);
  REGP_CONSTANT("PRINTER_PEN_DASHDOTDOT", PS_DASHDOTDOT);
  REGP_CONSTANT("PRINTER_PEN_INVISIBLE", PS_NULL);

  REGP_CONSTANT("PRINTER_BRUSH_SOLID", BRUSH_SOLID);
  REGP_CONSTANT("PRINTER_BRUSH_CUSTOM", BRUSH_CUSTOM);
  REGP_CONSTANT("PRINTER_BRUSH_DIAGONAL", HS_BDIAGONAL);
  REGP_CONSTANT("PRINTER_BRUSH_CROSS", HS_CROSS);
  REGP_CONSTANT("PRINTER_BRUSH_DIAGCROSS", HS_DIAGCROSS);
  REGP_CONSTANT("PRINTER_BRUSH_FDIAGONAL", HS_FDIAGONAL);
  REGP_CONSTANT("PRINTER_BRUSH_HORIZONTAL", HS_HORIZONTAL);
  REGP_CONSTANT("PRINTER_BRUSH_VERTICAL", HS_VERTICAL);

  REGP_CONSTANT("PRINTER_FW_THIN", FW_THIN);
  REGP_CONSTANT("PRINTER_FW_ULTRALIGHT", FW_ULTRALIGHT);
  REGP_CONSTANT("PRINTER_FW_LIGHT", FW_LIGHT);
  REGP_CONSTANT("PRINTER_FW_NORMAL", FW_NORMAL);
  REGP_CONSTANT("PRINTER_FW_MEDIUM", FW_MEDIUM);
  REGP_CONSTANT("PRINTER_FW_BOLD", FW_BOLD);
  REGP_CONSTANT("PRINTER_FW_ULTRABOLD", FW_ULTRABOLD);
  REGP_CONSTANT("PRINTER_FW_HEAVY", FW_HEAVY);

  REGP_CONSTANT("PRINTER_ENUM_LOCAL", PRINTER_ENUM_LOCAL);
  REGP_CONSTANT("PRINTER_ENUM_NAME", PRINTER_ENUM_NAME);
  REGP_CONSTANT("PRINTER_ENUM_SHARED", PRINTER_ENUM_SHARED);
  REGP_CONSTANT("PRINTER_ENUM_DEFAULT", PRINTER_ENUM_DEFAULT);
  REGP_CONSTANT("PRINTER_ENUM_CONNECTIONS", PRINTER_ENUM_CONNECTIONS);
  REGP_CONSTANT("PRINTER_ENUM_NETWORK", PRINTER_ENUM_NETWORK);
  REGP_CONSTANT("PRINTER_ENUM_REMOTE", PRINTER_ENUM_REMOTE);
#else
  /* Linux/Unix - minimal constants */
  REGP_CONSTANT("PRINTER_ENUM_LOCAL", 2);
  REGP_CONSTANT("PRINTER_ENUM_DEFAULT", 1);
#endif

  return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(printer) {
#ifdef ZTS
  ts_free_id(printer_globals_id);
#else
  php_printer_shutdown(&printer_globals);
#endif
  UNREGISTER_INI_ENTRIES();
  return SUCCESS;
}

/* {{{ proto mixed printer_open([string printername])
   Return a handle to the printer or false if connection failed */
PHP_FUNCTION(printer_open) {
  char *printername = NULL;
  size_t printername_len = 0;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|s", &printername,
                            &printername_len) == FAILURE) {
    RETURN_THROWS();
  }

  resource = (printer *)emalloc(sizeof(printer));

#ifdef PHP_WIN32
  resource->dmModifiedFields = 0;

  if (printername != NULL) {
    resource->name = printername;
  } else {
    resource->name = PRINTERG(default_printer);
  }

  if (OpenPrinter(resource->name, &resource->handle, NULL) != 0) {
    resource->pi2 = (PRINTER_INFO_2 *)emalloc(sizeof(PRINTER_INFO_2));
    resource->pi2->pDevMode = (DEVMODE *)emalloc(
        DocumentProperties(NULL, NULL, resource->name, NULL, NULL, 0));
    if (DocumentProperties(NULL, resource->handle, resource->name,
                           resource->pi2->pDevMode, NULL,
                           DM_OUT_BUFFER) == IDOK) {
      resource->info.lpszDocName = estrdup("PHP generated Document");
      resource->info.lpszOutput = NULL;
      resource->info.lpszDatatype = estrdup("TEXT");
      resource->info.fwType = 0;
      resource->info.cbSize = sizeof(resource->info);
      resource->dc =
          CreateDC(NULL, resource->name, NULL, resource->pi2->pDevMode);
      RETURN_RES(zend_register_resource(resource, le_printer));
    } else {
      /* Cleanup on DocumentProperties failure */
      efree(resource->pi2->pDevMode);
      efree(resource->pi2);
      ClosePrinter(resource->handle);
      {
        char *printer_name = resource->name;
        efree(resource);
        php_error_docref(NULL, E_WARNING, "couldn't configure printer [%s]",
                         printer_name);
        RETURN_FALSE;
      }
    }
  } else {
    php_error_docref(NULL, E_WARNING, "couldn't connect to the printer [%s]",
                     resource->name);
    efree(resource);
    RETURN_FALSE;
  }
#else
  /* Linux/Unix CUPS implementation */
#ifdef HAVE_CUPS
	cups_dest_t *dests;
	int num_dests;
	cups_dest_t *dest = NULL;
	
	if (printername != NULL) {
		resource->name = estrdup(printername);
	}
	else {
		resource->name = get_default_printer();
		if (!resource->name) {
			php_error_docref(NULL, E_WARNING, "no default printer found");
			efree(resource);
			RETURN_FALSE;
		}
	}
	
	num_dests = cupsGetDests(&dests);
	if (num_dests < 0) {
		php_error_docref(NULL, E_WARNING, "failed to get list of printers");
		efree(resource->name);
		efree(resource);
		RETURN_FALSE;
	}
	dest = cupsGetDest(resource->name, NULL, num_dests, dests);
	
	if (dest) {
		/* cupsCopyDest returns cups_dest_t* in CUPS 1.6+, int in older versions */
		#if defined(CUPS_VERSION_MAJOR) && (CUPS_VERSION_MAJOR >= 2 || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 6))
		resource->dest = cupsCopyDest(dest, 0, NULL);
		#else
		/* For older CUPS versions (or when version macros unavailable), manually copy the destination */
		resource->dest = (cups_dest_t *)emalloc(sizeof(cups_dest_t));
		if (!resource->dest) {
			cupsFreeDests(num_dests, dests);
			php_error_docref(NULL, E_WARNING, "failed to allocate memory for printer destination [%s]", resource->name);
			efree(resource->name);
			efree(resource);
			RETURN_FALSE;
		}
		
		/* Explicitly initialize fields to avoid shallow-copying internal pointers */
		resource->dest->name = NULL;
		resource->dest->instance = NULL;
		resource->dest->is_default = dest->is_default;
		resource->dest->num_options = 0;
		resource->dest->options = NULL;
		
		if (dest->name) {
			resource->dest->name = strdup(dest->name);
			if (!resource->dest->name) {
				efree(resource->dest);
				cupsFreeDests(num_dests, dests);
				php_error_docref(NULL, E_WARNING, "failed to allocate memory for printer name [%s]", resource->name);
				efree(resource->name);
				efree(resource);
				RETURN_FALSE;
			}
		}
		if (dest->instance) {
			resource->dest->instance = strdup(dest->instance);
			if (!resource->dest->instance) {
				if (resource->dest->name) {
					free(resource->dest->name);
				}
				efree(resource->dest);
				cupsFreeDests(num_dests, dests);
				php_error_docref(NULL, E_WARNING, "failed to allocate memory for printer instance [%s]", resource->name);
				efree(resource->name);
				efree(resource);
				RETURN_FALSE;
			}
		}
		
		/* Copy printer options using CUPS API to preserve configuration safely */
		if (dest->num_options > 0 && dest->options) {
			/* cupsCopyOptions allocates memory for the options array using CUPS routines,
			 * which must later be freed via cupsFreeOptions/cupsFreeDests. The dest->name
			 * and dest->instance fields above are allocated via strdup() (malloc-based)
			 * but remain compatible with cupsFreeDests, which uses standard free(). */
			resource->dest->num_options = cupsCopyOptions(
				dest->num_options,
				dest->options,
				0,
				&resource->dest->options
			);
			if (resource->dest->num_options == 0 || !resource->dest->options) {
				/* Clean up allocated options if any */
				if (resource->dest->options) {
					cupsFreeOptions(resource->dest->num_options, resource->dest->options);
				}
				if (resource->dest->name) {
					free(resource->dest->name);
				}
				if (resource->dest->instance) {
					free(resource->dest->instance);
				}
				efree(resource->dest);
				cupsFreeDests(num_dests, dests);
				php_error_docref(NULL, E_WARNING, "failed to copy printer options for [%s]", resource->name);
				efree(resource->name);
				efree(resource);
				RETURN_FALSE;
			}
		}
		#endif
		if (!resource->dest) {
			cupsFreeDests(num_dests, dests);
			php_error_docref(NULL, E_WARNING, "failed to copy printer destination for [%s]", resource->name);
			efree(resource->name);
			efree(resource);
			RETURN_FALSE;
		}
		resource->http = NULL;
		resource->job_id = 0;
		resource->title = estrdup("PHP generated Document");
		resource->datatype = estrdup("application/octet-stream");
		resource->num_options = 0;
		resource->options = NULL;
		
		cupsFreeDests(num_dests, dests);
		RETURN_RES(zend_register_resource(resource, le_printer));
	}
	else {
		cupsFreeDests(num_dests, dests);
		php_error_docref(NULL, E_WARNING, "couldn't connect to the printer [%s]", resource->name);
		efree(resource->name);
		efree(resource);
		RETURN_FALSE;
	}
#else
    /* For older CUPS versions (or when version macros unavailable), manually
     * copy the destination */
    resource->dest = (cups_dest_t *)emalloc(sizeof(cups_dest_t));
    if (!resource->dest) {
      cupsFreeDests(num_dests, dests);
      php_error_docref(NULL, E_WARNING,
                       "failed to allocate memory for printer destination [%s]",
                       resource->name);
      efree(resource->name);
      efree(resource);
      RETURN_FALSE;
    }

    /* Explicitly initialize fields to avoid shallow-copying internal pointers
     */
    resource->dest->name = NULL;
    resource->dest->instance = NULL;
    resource->dest->is_default = dest->is_default;
    resource->dest->num_options = 0;
    resource->dest->options = NULL;

    if (dest->name) {
      resource->dest->name = strdup(dest->name);
      if (!resource->dest->name) {
        efree(resource->dest);
        cupsFreeDests(num_dests, dests);
        php_error_docref(NULL, E_WARNING,
                         "failed to allocate memory for printer name [%s]",
                         resource->name);
        efree(resource->name);
        efree(resource);
        RETURN_FALSE;
      }
    }
    if (dest->instance) {
      resource->dest->instance = strdup(dest->instance);
      if (!resource->dest->instance) {
        if (resource->dest->name) {
          free(resource->dest->name);
        }
        efree(resource->dest);
        cupsFreeDests(num_dests, dests);
        php_error_docref(NULL, E_WARNING,
                         "failed to allocate memory for printer instance [%s]",
                         resource->name);
        efree(resource->name);
        efree(resource);
        RETURN_FALSE;
      }
    }

    /* Copy printer options using CUPS API to preserve configuration safely */
    if (dest->num_options > 0 && dest->options) {
      /* cupsCopyOptions allocates memory for the options array using CUPS
       * routines, which must later be freed via cupsFreeOptions/cupsFreeDests.
       * The dest->name and dest->instance fields above are allocated via
       * strdup() (malloc-based) but remain compatible with cupsFreeDests, which
       * uses standard free(). */
      resource->dest->num_options = cupsCopyOptions(
          dest->num_options, dest->options, 0, &resource->dest->options);
      if (resource->dest->num_options == 0 || !resource->dest->options) {
        /* Clean up allocated options if any */
        if (resource->dest->options) {
          cupsFreeOptions(resource->dest->num_options, resource->dest->options);
        }
        if (resource->dest->name) {
          free(resource->dest->name);
        }
        if (resource->dest->instance) {
          free(resource->dest->instance);
        }
        efree(resource->dest);
        cupsFreeDests(num_dests, dests);
        php_error_docref(NULL, E_WARNING,
                         "failed to copy printer options for [%s]",
                         resource->name);
        efree(resource->name);
        efree(resource);
        RETURN_FALSE;
      }
    }
#endif
    if (!resource->dest) {
      cupsFreeDests(num_dests, dests);
      php_error_docref(NULL, E_WARNING,
                       "failed to copy printer destination for [%s]",
                       resource->name);
      efree(resource->name);
      efree(resource);
      RETURN_FALSE;
    }
    resource->http = NULL;
    resource->job_id = 0;
    resource->title = estrdup("PHP generated Document");
    resource->datatype = estrdup("application/octet-stream");
    resource->num_options = 0;
    resource->options = NULL;

    cupsFreeDests(num_dests, dests);
    RETURN_RES(zend_register_resource(resource, le_printer));
  } else {
    cupsFreeDests(num_dests, dests);
    php_error_docref(NULL, E_WARNING, "couldn't connect to the printer [%s]",
                     resource->name);
    efree(resource->name);
    efree(resource);
    RETURN_FALSE;
  }
#else
  php_error_docref(NULL, E_WARNING, "CUPS support not compiled in");
  efree(resource);
  RETURN_FALSE;
#endif /* HAVE_CUPS */
#endif /* PHP_WIN32 */
}
/* }}} */

/* {{{ proto void printer_close(resource connection)
   Close the printer connection */
PHP_FUNCTION(printer_close) {
  zval *zres;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zres) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  zend_list_close(Z_RES_P(zres));
}
/* }}} */

/* {{{ proto bool printer_write(resource connection,string content)
   Write directly to the printer */
PHP_FUNCTION(printer_write) {
  zval *zres;
  char *content;
  size_t content_len;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &zres, &content,
                            &content_len) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

#ifdef PHP_WIN32
  DOC_INFO_1 docinfo;
  int sd, sp = 0, received;

  docinfo.pDocName = (LPTSTR)resource->info.lpszDocName;
  docinfo.pOutputFile = (LPTSTR)resource->info.lpszOutput;
  docinfo.pDatatype = (LPTSTR)resource->info.lpszDatatype;

  sd = StartDocPrinter(resource->handle, 1, (LPBYTE)&docinfo);
  sp = StartPagePrinter(resource->handle);

  if (sd && sp) {
    WritePrinter(resource->handle, content, content_len, &received);
    EndPagePrinter(resource->handle);
    EndDocPrinter(resource->handle);
    RETURN_TRUE;
  } else {
    php_error_docref(NULL, E_WARNING, "couldn't allocate the printerjob [%d]",
                     GetLastError());
    RETURN_FALSE;
  }
#else
  /* Linux/Unix CUPS implementation */
#ifdef HAVE_CUPS
  int job_id;

  /* Create a print job and submit content */
  job_id =
      cupsCreateJob(CUPS_HTTP_DEFAULT, resource->dest->name, resource->title,
                    resource->num_options, resource->options);

  if (job_id == 0) {
    php_error_docref(NULL, E_WARNING, "couldn't create print job: %s",
                     cupsLastErrorString());
    RETURN_FALSE;
  }

  resource->job_id = job_id;

  if (cupsStartDocument(CUPS_HTTP_DEFAULT, resource->dest->name, job_id,
                        resource->title, CUPS_FORMAT_RAW, 1) != HTTP_CONTINUE) {
    php_error_docref(NULL, E_WARNING, "couldn't start document: %s",
                     cupsLastErrorString());
    if (resource->dest && resource->dest->name) {
      cupsCancelJob(resource->dest->name, job_id);
    }
    RETURN_FALSE;
  }

  if (cupsWriteRequestData(CUPS_HTTP_DEFAULT, content, content_len) !=
      HTTP_CONTINUE) {
    php_error_docref(NULL, E_WARNING, "couldn't write data: %s",
                     cupsLastErrorString());
    if (resource->dest && resource->dest->name) {
      cupsCancelJob(resource->dest->name, job_id);
    }
    RETURN_FALSE;
  }

  if (cupsFinishDocument(CUPS_HTTP_DEFAULT, resource->dest->name) != IPP_OK) {
    php_error_docref(NULL, E_WARNING, "couldn't finish document: %s",
                     cupsLastErrorString());
    if (resource->dest && resource->dest->name) {
      cupsCancelJob(resource->dest->name, job_id);
    }
    RETURN_FALSE;
  }

  RETURN_TRUE;
#else
  php_error_docref(NULL, E_WARNING, "CUPS support not compiled in");
  RETURN_FALSE;
#endif /* HAVE_CUPS */
#endif /* PHP_WIN32 */
}
/* }}} */

/* {{{ proto array printer_list(int EnumType [, string Name [, int Level]])
   Return an array of printers attached to the server */
PHP_FUNCTION(printer_list) {
#ifdef PHP_WIN32
  zval Printer;
  char InfoBuffer[8192], *Name = NULL;
  size_t Name_len = 0;
  zend_long enumtype, Level = 1;

  PRINTER_INFO_1 *P1;
  PRINTER_INFO_2 *P2;
  PRINTER_INFO_4 *P4;
  PRINTER_INFO_5 *P5;
  DWORD bNeeded = sizeof(InfoBuffer), cReturned, i;
  int LevvelsAllowed[] = {0, 1, 1, 0, 1, 1};

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|sl", &enumtype, &Name,
                            &Name_len, &Level) == FAILURE) {
    RETURN_THROWS();
  }

  if (!LevvelsAllowed[Level]) {
    php_error_docref(NULL, E_WARNING, "Level not allowed");
    RETURN_FALSE;
  }

  array_init(return_value);

  EnumPrinters(enumtype, Name, Level, (LPBYTE)InfoBuffer, sizeof(InfoBuffer),
               &bNeeded, &cReturned);

  P1 = (PRINTER_INFO_1 *)InfoBuffer;
  P2 = (PRINTER_INFO_2 *)InfoBuffer;
  P4 = (PRINTER_INFO_4 *)InfoBuffer;
  P5 = (PRINTER_INFO_5 *)InfoBuffer;

  for (i = 0; i < cReturned; i++) {
    array_init(&Printer);

    switch (Level) {
    case 1:
      add_assoc_string(&Printer, "NAME", P1->pName);
      add_assoc_string(&Printer, "DESCRIPTION", P1->pDescription);
      add_assoc_string(&Printer, "COMMENT", P1->pComment);
      P1++;
      break;

    case 2:
      if (P2->pServerName)
        add_assoc_string(&Printer, "SERVERNAME", P2->pServerName);
      if (P2->pPrinterName)
        add_assoc_string(&Printer, "PRINTERNAME", P2->pPrinterName);
      if (P2->pShareName)
        add_assoc_string(&Printer, "SHARENAME", P2->pShareName);
      if (P2->pPortName)
        add_assoc_string(&Printer, "PORTNAME", P2->pPortName);
      if (P2->pDriverName)
        add_assoc_string(&Printer, "DRIVERNAME", P2->pDriverName);
      if (P2->pComment)
        add_assoc_string(&Printer, "COMMENT", P2->pComment);
      if (P2->pLocation)
        add_assoc_string(&Printer, "LOCATION", P2->pLocation);
      if (P2->pSepFile)
        add_assoc_string(&Printer, "SEPFILE", P2->pSepFile);
      if (P2->pPrintProcessor)
        add_assoc_string(&Printer, "PRINTPROCESSOR", P2->pPrintProcessor);
      if (P2->pDatatype)
        add_assoc_string(&Printer, "DATATYPE", P2->pDatatype);
      if (P2->pParameters)
        add_assoc_string(&Printer, "PARAMETRES", P2->pParameters);
      add_assoc_long(&Printer, "ATTRIBUTES", P2->Attributes);
      add_assoc_long(&Printer, "PRIORITY", P2->Priority);
      add_assoc_long(&Printer, "DEFAULTPRIORITY", P2->DefaultPriority);
      add_assoc_long(&Printer, "STARTTIME", P2->StartTime);
      add_assoc_long(&Printer, "UNTILTIME", P2->UntilTime);
      add_assoc_long(&Printer, "STATUS", P2->Status);
      add_assoc_long(&Printer, "CJOBS", P2->cJobs);
      add_assoc_long(&Printer, "AVERAGEPPM", P2->AveragePPM);
      P2++;
      break;

    case 4:
      add_assoc_string(&Printer, "PRINTERNAME", P4->pPrinterName);
      add_assoc_string(&Printer, "SERVERNAME", P4->pServerName);
      add_assoc_long(&Printer, "ATTRIBUTES", P4->Attributes);
      P4++;
      break;

    case 5:
      add_assoc_string(&Printer, "PRINTERNAME", P5->pPrinterName);
      add_assoc_string(&Printer, "PORTNAME", P5->pPortName);
      add_assoc_long(&Printer, "ATTRIBUTES", P5->Attributes);
      add_assoc_long(&Printer, "DEVICENOTSELECTEDTIMEOUT",
                     P5->DeviceNotSelectedTimeout);
      add_assoc_long(&Printer, "TRANSMISSIONRETRYTIMEOUT",
                     P5->TransmissionRetryTimeout);
      P5++;
      break;
    }

    add_index_zval(return_value, i, &Printer);
  }
#else
  /* Linux/Unix CUPS implementation */
#ifdef HAVE_CUPS
  zval Printer;
  cups_dest_t *dests;
  int num_dests;
  int i;
  int j;
  int array_index;
  zend_long enumtype;
  char *Name = NULL;
  size_t Name_len = 0;
  zend_long Level = 1;

  /* Parse parameters for compatibility */
  if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|sl", &enumtype, &Name,
                            &Name_len, &Level) == FAILURE) {
    RETURN_THROWS();
  }

  /* Validate Level parameter - CUPS supports levels 1 and 2 */
  if (Level != 1 && Level != 2) {
    php_error_docref(NULL, E_WARNING,
                     "Level not allowed (CUPS supports levels 1 and 2)");
    RETURN_FALSE;
  }

  array_init(return_value);

  num_dests = cupsGetDests(&dests);
  if (num_dests < 0) {
    php_error_docref(NULL, E_WARNING, "failed to get list of printers");
    RETURN_FALSE;
  }

  array_index = 0;
  for (i = 0; i < num_dests; i++) {
    /* Filter based on enumtype for better Windows compatibility */
    if (enumtype == PRINTER_ENUM_DEFAULT && !dests[i].is_default) {
      continue;
    }

    /* Filter by name if provided */
    if (Name != NULL && Name_len > 0) {
      if (strcmp(dests[i].name, Name) != 0) {
        continue;
      }
    }

    array_init(&Printer);

    /* Level 1: Basic information */
    add_assoc_string(&Printer, "NAME", dests[i].name);

    if (dests[i].instance) {
      add_assoc_string(&Printer, "INSTANCE", dests[i].instance);
    }

    add_assoc_bool(&Printer, "IS_DEFAULT", dests[i].is_default);

    /* Level 2: Include printer options/attributes */
    if (Level >= 2) {
      for (j = 0; j < dests[i].num_options; j++) {
        add_assoc_string(&Printer, dests[i].options[j].name,
                         dests[i].options[j].value);
      }
    }

    add_index_zval(return_value, array_index, &Printer);
    array_index++;
  }

  cupsFreeDests(num_dests, dests);
#else
  php_error_docref(NULL, E_WARNING, "CUPS support not compiled in");
  RETURN_FALSE;
#endif /* HAVE_CUPS */
#endif /* PHP_WIN32 */
}

/* }}} */

#ifdef PHP_WIN32

/* {{{ proto bool printer_set_option(resource connection,string option,mixed
   value) Configure the printer device */
PHP_FUNCTION(printer_set_option) {
  DWORD dwNeeded = 0;
  PRINTER_DEFAULTS pd;
  zval *zres, *value;
  zend_long option;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlz", &zres, &option, &value) ==
      FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  memset(&pd, 0, sizeof(pd));
  pd.DesiredAccess = PRINTER_ALL_ACCESS;
  SetLastError(0);

  switch (option) {
  case COPIES:
    convert_to_long(value);
    resource->pi2->pDevMode->dmCopies = (short)Z_LVAL_P(value);
    resource->dmModifiedFields |= DM_COPIES;
    break;

  case MODE:
    convert_to_string(value);
    if (resource->info.lpszDatatype) {
      efree((char *)resource->info.lpszDatatype);
    }
    resource->info.lpszDatatype = estrdup(Z_STRVAL_P(value));
    resource->info.cbSize = sizeof(resource->info);
    break;

  case TITLE:
    convert_to_string(value);
    if (resource->info.lpszDocName) {
      efree((char *)resource->info.lpszDocName);
    }
    resource->info.lpszDocName = estrdup(Z_STRVAL_P(value));
    resource->info.cbSize = sizeof(resource->info);
    break;

  case OUTPUT_FILE:
    convert_to_string(value);
    if (resource->info.lpszOutput) {
      efree((char *)resource->info.lpszOutput);
    }
    resource->info.lpszOutput = estrdup(Z_STRVAL_P(value));
    resource->info.cbSize = sizeof(resource->info);
    break;

  case ORIENTATION:
    convert_to_long(value);
    resource->pi2->pDevMode->dmOrientation = (short)Z_LVAL_P(value);
    resource->dmModifiedFields |= DM_ORIENTATION;
    break;

  case YRESOLUTION:
    convert_to_long(value);
    resource->pi2->pDevMode->dmYResolution = (short)Z_LVAL_P(value);
    resource->dmModifiedFields |= DM_YRESOLUTION;
    break;

  case XRESOLUTION:
    convert_to_long(value);
    resource->pi2->pDevMode->dmPrintQuality = (short)Z_LVAL_P(value);
    resource->dmModifiedFields |= DM_PRINTQUALITY;
    break;

  case PAPER_FORMAT:
    convert_to_long(value);
    resource->pi2->pDevMode->dmPaperSize = (short)Z_LVAL_P(value);
    resource->dmModifiedFields |= DM_PAPERSIZE;
    break;

  case PAPER_LENGTH:
    convert_to_long(value);
    resource->pi2->pDevMode->dmPaperLength = (short)(Z_LVAL_P(value) * 10);
    resource->dmModifiedFields |= DM_PAPERLENGTH;
    break;

  case PAPER_WIDTH:
    convert_to_long(value);
    resource->pi2->pDevMode->dmPaperWidth = (short)(Z_LVAL_P(value) * 10);
    resource->dmModifiedFields |= DM_PAPERWIDTH;
    break;

  case SCALE:
    convert_to_long(value);
    resource->pi2->pDevMode->dmScale = (short)Z_LVAL_P(value);
    resource->dmModifiedFields |= DM_SCALE;
    break;

  case BG_COLOR:
    convert_to_string(value);
    SetBkColor(resource->dc, hex_to_rgb(Z_STRVAL_P(value)));
    break;

  case TEXT_COLOR:
    convert_to_string(value);
    SetTextColor(resource->dc, hex_to_rgb(Z_STRVAL_P(value)));
    break;

  case TEXT_ALIGN:
    convert_to_long(value);
    SetTextAlign(resource->dc, Z_LVAL_P(value));
    break;
  case VALID_OPTIONS:
    resource->pi2->pSecurityDescriptor = NULL;
    resource->pi2->pDevMode->dmFields = resource->dmModifiedFields;
    resource->dmModifiedFields = 0;
    DocumentProperties(NULL, resource->handle, resource->name,
                       resource->pi2->pDevMode, resource->pi2->pDevMode,
                       DM_IN_BUFFER | DM_OUT_BUFFER);
    SetPrinter(resource->handle, 2, (LPBYTE)resource->pi2, 0);
    SendMessageTimeout(HWND_BROADCAST, WM_DEVMODECHANGE, 0L,
                       (LPARAM)(LPCSTR)resource->name, SMTO_NORMAL, 1000, NULL);
    break;
  default:
    php_error_docref(NULL, E_WARNING,
                     "unknown option passed to printer_set_option()");
    RETURN_FALSE;
  }

  RETURN_TRUE;
}
/* }}} */

/* {{{ proto mixed printer_get_option(int handle, string option)
   Get configured data */
PHP_FUNCTION(printer_get_option) {
  zval *zres;
  zend_long option;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &zres, &option) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  switch (option) {
  case COPIES:
    RETURN_LONG(resource->pi2->pDevMode->dmCopies);

  case MODE:
    RETURN_STRING((char *)resource->info.lpszDatatype);

  case TITLE:
    RETURN_STRING((char *)resource->info.lpszDocName);

  case OUTPUT_FILE:
    if (resource->info.lpszOutput) {
      RETURN_STRING((char *)resource->info.lpszOutput);
    } else {
      RETURN_NULL();
    }

  case ORIENTATION:
    RETURN_LONG(resource->pi2->pDevMode->dmOrientation);

  case YRESOLUTION:
    RETURN_LONG(resource->pi2->pDevMode->dmYResolution);

  case XRESOLUTION:
    RETURN_LONG(resource->pi2->pDevMode->dmPrintQuality);

  case PAPER_FORMAT:
    RETURN_LONG(resource->pi2->pDevMode->dmPaperSize);

  case PAPER_LENGTH:
    RETURN_LONG(resource->pi2->pDevMode->dmPaperLength / 10);

  case PAPER_WIDTH:
    RETURN_LONG(resource->pi2->pDevMode->dmPaperWidth / 10);

  case SCALE:
    RETURN_LONG(resource->pi2->pDevMode->dmScale);

  case BG_COLOR:
    RETURN_STRING(rgb_to_hex(GetBkColor(resource->dc)));

  case TEXT_COLOR:
    RETURN_STRING(rgb_to_hex(GetTextColor(resource->dc)));

  case TEXT_ALIGN:
    RETURN_LONG(GetTextAlign(resource->dc));

  case DEVICENAME:
    RETURN_STRING(resource->name);

  case DRIVER_VERSION:
    RETURN_LONG(resource->pi2->pDevMode->dmDriverVersion);

  default:
    php_error_docref(NULL, E_WARNING,
                     "unknown option passed to printer_get_option()");
    RETURN_FALSE;
  }
}
/* }}} */

/* {{{ proto void printer_create_dc(int handle)
   Create a device content */
PHP_FUNCTION(printer_create_dc) {
  zval *zres;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zres) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  if (resource->dc != NULL) {
    php_error_docref(NULL, E_WARNING, "Deleting old DeviceContext");
    DeleteDC(resource->dc);
  }

  resource->dc = CreateDC(NULL, resource->name, NULL, resource->pi2->pDevMode);
}
/* }}} */

/* {{{ proto bool printer_delete_dc(int handle)
   Delete a device content */
PHP_FUNCTION(printer_delete_dc) {
  zval *zres;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zres) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  if (resource->dc != NULL) {
    DeleteDC(resource->dc);
    resource->dc = NULL;
    RETURN_TRUE;
  } else {
    php_error_docref(NULL, E_WARNING, "No DeviceContext created");
    RETURN_FALSE;
  }
}
/* }}} */

/* {{{ proto bool printer_start_doc(int handle)
   Start a document */
PHP_FUNCTION(printer_start_doc) {
  zval *zres;
  char *document = NULL;
  size_t document_len = 0;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "r|s", &zres, &document,
                            &document_len) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  if (document != NULL) {
    if (resource->info.lpszDocName) {
      efree((char *)resource->info.lpszDocName);
    }
    resource->info.lpszDocName = estrdup(document);
    resource->info.cbSize = sizeof(resource->info);
  }

  if (StartDoc(resource->dc, &resource->info) < 0) {
    php_error_docref(NULL, E_WARNING, "couldn't allocate new print job");
    RETURN_FALSE;
  }

  RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool printer_end_doc(int handle)
   End a document */
PHP_FUNCTION(printer_end_doc) {
  zval *zres;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zres) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  if (EndDoc(resource->dc) < 0) {
    php_error_docref(NULL, E_WARNING, "couldn't terminate print job");
    RETURN_FALSE;
  }

  RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool printer_start_page(int handle)
   Start a page */
PHP_FUNCTION(printer_start_page) {
  zval *zres;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zres) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  if (StartPage(resource->dc) < 0) {
    php_error_docref(NULL, E_WARNING, "couldn't start a new page");
    RETURN_FALSE;
  }

  RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool printer_end_page(int handle)
   End a page */
PHP_FUNCTION(printer_end_page) {
  zval *zres;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zres) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  if (EndPage(resource->dc) < 0) {
    php_error_docref(NULL, E_WARNING, "couldn't end the page");
    RETURN_FALSE;
  }

  RETURN_TRUE;
}
/* }}} */

/* {{{ proto mixed printer_create_pen(int style, int width, string color)
   Create a pen */
PHP_FUNCTION(printer_create_pen) {
  zend_long style, width;
  char *color;
  size_t color_len;
  HPEN pen;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "lls", &style, &width, &color,
                            &color_len) == FAILURE) {
    RETURN_THROWS();
  }

  pen = CreatePen(style, width, hex_to_rgb(color));

  if (!pen) {
    RETURN_FALSE;
  }

  RETURN_RES(zend_register_resource(pen, le_pen));
}
/* }}} */

/* {{{ proto void printer_delete_pen(resource pen_handle)
   Delete a pen */
PHP_FUNCTION(printer_delete_pen) {
  zval *zres;
  HPEN pen;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zres) == FAILURE) {
    RETURN_THROWS();
  }

  if ((pen = (HPEN)zend_fetch_resource(Z_RES_P(zres), "Pen Handle", le_pen)) ==
      NULL) {
    RETURN_THROWS();
  }

  zend_list_close(Z_RES_P(zres));
}
/* }}} */

/* {{{ proto void printer_select_pen(resource printer_handle, resource
   pen_handle) Select a pen */
PHP_FUNCTION(printer_select_pen) {
  zval *zres_printer, *zres_pen;
  HPEN pen;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &zres_printer, &zres_pen) ==
      FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres_printer), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }
  if ((pen = (HPEN)zend_fetch_resource(Z_RES_P(zres_pen), "Pen Handle",
                                       le_pen)) == NULL) {
    RETURN_THROWS();
  }

  SelectObject(resource->dc, pen);
}
/* }}} */

/* {{{ proto mixed printer_create_brush(int style, string color_or_file)
   Create a brush - color_or_file is a hex color for solid/hatch brushes, or
   bitmap path for custom brushes */
PHP_FUNCTION(printer_create_brush) {
  zend_long style;
  char *value;
  size_t value_len;
  HBRUSH brush;
  HBITMAP bmp;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "ls", &style, &value,
                            &value_len) == FAILURE) {
    RETURN_THROWS();
  }

  switch (style) {
  case BRUSH_SOLID:
    brush = CreateSolidBrush(hex_to_rgb(value));
    break;
  case BRUSH_CUSTOM:
    bmp = LoadImage(0, value, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    brush = CreatePatternBrush(bmp);
    break;
  default:
    brush = CreateHatchBrush(style, hex_to_rgb(value));
  }

  if (!brush) {
    RETURN_FALSE;
  }

  RETURN_RES(zend_register_resource(brush, le_brush));
}
/* }}} */

/* {{{ proto void printer_delete_brush(resource brush_handle)
   Delete a brush */
PHP_FUNCTION(printer_delete_brush) {
  zval *zres;
  HBRUSH brush;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zres) == FAILURE) {
    RETURN_THROWS();
  }

  if ((brush = (HBRUSH)zend_fetch_resource(Z_RES_P(zres), "Brush Handle",
                                           le_brush)) == NULL) {
    RETURN_THROWS();
  }

  zend_list_close(Z_RES_P(zres));
}
/* }}} */

/* {{{ proto void printer_select_brush(resource printer_handle, resource
   brush_handle) Select a brush */
PHP_FUNCTION(printer_select_brush) {
  zval *zres_printer, *zres_brush;
  HBRUSH brush;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &zres_printer,
                            &zres_brush) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres_printer), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }
  if ((brush = (HBRUSH)zend_fetch_resource(Z_RES_P(zres_brush), "Brush Handle",
                                           le_brush)) == NULL) {
    RETURN_THROWS();
  }

  SelectObject(resource->dc, brush);
}
/* }}} */

/* {{{ proto mixed printer_create_font(string face, int height, int width, int
   font_weight, bool italic, bool underline, bool strikeout, int orientaton)
   Create a font */
PHP_FUNCTION(printer_create_font) {
  char *face_param;
  size_t face_len;
  zend_long height, width, font_weight, orientation;
  bool italic, underline, strikeout;
  HFONT font;
  char face[33];

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "sllllbbl", &face_param, &face_len,
                            &height, &width, &font_weight, &italic, &underline,
                            &strikeout, &orientation) == FAILURE) {
    RETURN_THROWS();
  }

  /* Copy up to 32 chars for face name */
  strncpy(face, face_param, 32);
  face[32] = '\0';

  font = CreateFont(height, width, orientation, orientation, font_weight,
                    italic, underline, strikeout, DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_ROMAN, face);

  if (!font) {
    RETURN_FALSE;
  }

  RETURN_RES(zend_register_resource(font, le_font));
}
/* }}} */

/* {{{ proto void printer_delete_font(int fonthandle)
   Delete a font */
PHP_FUNCTION(printer_delete_font) {
  zval *zres;
  HFONT font;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zres) == FAILURE) {
    RETURN_THROWS();
  }

  if ((font = (HFONT)zend_fetch_resource(Z_RES_P(zres), "Font Handle",
                                         le_font)) == NULL) {
    RETURN_THROWS();
  }

  zend_list_close(Z_RES_P(zres));
}
/* }}} */

/* {{{ proto void printer_select_font(int printerhandle, int fonthandle)
   Select a font */
PHP_FUNCTION(printer_select_font) {
  zval *zres_printer, *zres_font;
  HFONT font;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &zres_printer, &zres_font) ==
      FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres_printer), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }
  if ((font = (HFONT)zend_fetch_resource(Z_RES_P(zres_font), "Font Handle",
                                         le_font)) == NULL) {
    RETURN_THROWS();
  }

  SelectObject(resource->dc, font);
}
/* }}} */

/* {{{ proto int printer_logical_fontheight(int handle, int height)
   Get the logical font height */
PHP_FUNCTION(printer_logical_fontheight) {
  zval *zres;
  zend_long height;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &zres, &height) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  RETURN_LONG(MulDiv(height, GetDeviceCaps(resource->dc, LOGPIXELSY), 72));
}
/* }}} */

/* {{{ proto void printer_draw_roundrect(resource handle, int ul_x, int ul_y,
   int lr_x, int lr_y, int width, int height) Draw a roundrect */
PHP_FUNCTION(printer_draw_roundrect) {
  zval *zres;
  zend_long ul_x, ul_y, lr_x, lr_y, width, height;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllllll", &zres, &ul_x, &ul_y,
                            &lr_x, &lr_y, &width, &height) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  RoundRect(resource->dc, ul_x, ul_y, lr_x, lr_y, width, height);
}
/* }}} */

/* {{{ proto void printer_draw_rectangle(resource handle, int ul_x, int ul_y,
   int lr_x, int lr_y) Draw a rectangle */
PHP_FUNCTION(printer_draw_rectangle) {
  zval *zres;
  zend_long ul_x, ul_y, lr_x, lr_y;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll", &zres, &ul_x, &ul_y,
                            &lr_x, &lr_y) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  Rectangle(resource->dc, ul_x, ul_y, lr_x, lr_y);
}
/* }}} */

/* {{{ proto void printer_draw_elipse(resource handle, int ul_x, int ul_y, int
   lr_x, int lr_y) Draw an elipse */
PHP_FUNCTION(printer_draw_elipse) {
  zval *zres;
  zend_long ul_x, ul_y, lr_x, lr_y;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll", &zres, &ul_x, &ul_y,
                            &lr_x, &lr_y) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  Ellipse(resource->dc, ul_x, ul_y, lr_x, lr_y);
}
/* }}} */

/* {{{ proto void printer_draw_text(resource handle, string text, int x, int y)
   Draw text */
PHP_FUNCTION(printer_draw_text) {
  zval *zres;
  char *text;
  size_t text_len;
  zend_long x, y;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rsll", &zres, &text, &text_len,
                            &x, &y) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  ExtTextOut(resource->dc, x, y, ETO_OPAQUE, NULL, text, text_len, NULL);
}
/* }}} */

/* {{{ proto void printer_draw_line(int handle, int fx, int fy, int tx, int ty)
   Draw line from x, y to x, y*/
PHP_FUNCTION(printer_draw_line) {
  zval *zres;
  zend_long fx, fy, tx, ty;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll", &zres, &fx, &fy, &tx,
                            &ty) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  MoveToEx(resource->dc, fx, fy, NULL);
  LineTo(resource->dc, tx, ty);
}
/* }}} */

/* {{{ proto void printer_draw_chord(resource handle, int rec_x, int rec_y, int
   rec_x1, int rec_y1, int rad_x, int rad_y, int rad_x1, int rad_y1) Draw a
   chord*/
PHP_FUNCTION(printer_draw_chord) {
  zval *zres;
  zend_long rec_x, rec_y, rec_x1, rec_y1, rad_x, rad_y, rad_x1, rad_y1;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllllllll", &zres, &rec_x, &rec_y,
                            &rec_x1, &rec_y1, &rad_x, &rad_y, &rad_x1,
                            &rad_y1) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  Chord(resource->dc, rec_x, rec_y, rec_x1, rec_y1, rad_x, rad_y, rad_x1,
        rad_y1);
}
/* }}} */

/* {{{ proto void printer_draw_pie(resource handle, int rec_x, int rec_y, int
   rec_x1, int rec_y1, int rad1_x, int rad1_y, int rad2_x, int rad2_y) Draw a
   pie*/
PHP_FUNCTION(printer_draw_pie) {
  zval *zres;
  zend_long rec_x, rec_y, rec_x1, rec_y1, rad1_x, rad1_y, rad2_x, rad2_y;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllllllll", &zres, &rec_x, &rec_y,
                            &rec_x1, &rec_y1, &rad1_x, &rad1_y, &rad2_x,
                            &rad2_y) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  Pie(resource->dc, rec_x, rec_y, rec_x1, rec_y1, rad1_x, rad1_y, rad2_x,
      rad2_y);
}
/* }}} */

/* {{{ proto mixed printer_draw_bmp(resource handle, string filename, int x, int
   y [, int width, int height]) Draw a bitmap */
PHP_FUNCTION(printer_draw_bmp) {
  zval *zres;
  char *filename;
  size_t filename_len;
  zend_long x, y, width = 0, height = 0;
  printer *resource;
  HBITMAP hbmp;
  BITMAP bmp_property;
  HDC dummy;
  int has_size;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "rsll|ll", &zres, &filename,
                            &filename_len, &x, &y, &width,
                            &height) == FAILURE) {
    RETURN_THROWS();
  }

  has_size = (ZEND_NUM_ARGS() == 6);

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

  hbmp = (HBITMAP)LoadImage(0, filename, IMAGE_BITMAP, 0, 0,
                            LR_CREATEDIBSECTION | LR_LOADFROMFILE);

  if (hbmp == NULL) {
    php_error_docref(NULL, E_WARNING, "Failed to load bitmap %s", filename);
    RETURN_FALSE;
  }

  if (GetObject(hbmp, sizeof(BITMAP), &bmp_property) == 0) {
    if (hbmp)
      DeleteObject(hbmp);
    RETURN_FALSE;
  }

  if (!(GetDeviceCaps(resource->dc, RASTERCAPS) & RC_STRETCHBLT)) {
    php_error_docref(NULL, E_WARNING, "Printer does not support bitmaps");
    DeleteObject(hbmp);
    RETURN_FALSE;
  }

  if ((dummy = CreateCompatibleDC(resource->dc)) == NULL) {
    DeleteObject(hbmp);
    RETURN_FALSE;
  }

  if (SelectObject(dummy, hbmp) == NULL) {
    DeleteDC(dummy);
    DeleteObject(hbmp);
    RETURN_FALSE;
  }

  if (has_size) {
    if (!StretchBlt(resource->dc, (int)x, (int)y, (int)width, (int)height,
                    dummy, 0, 0, bmp_property.bmWidth, bmp_property.bmHeight,
                    SRCCOPY)) {
      php_error_docref(NULL, E_WARNING, "Printer failed to accept bitmap");
      DeleteDC(dummy);
      DeleteObject(hbmp);
      RETURN_FALSE;
    }
  } else {
    if (!BitBlt(resource->dc, x, y, bmp_property.bmWidth, bmp_property.bmHeight,
                dummy, 0, 0, SRCCOPY)) {
      php_error_docref(NULL, E_WARNING, "Printer failed to accept bitmap");
      DeleteDC(dummy);
      DeleteObject(hbmp);
      RETURN_FALSE;
    }
  }

  DeleteDC(dummy);
  DeleteObject(hbmp);
  RETURN_TRUE;
}
/* }}} */

#else /* Not PHP_WIN32 - Linux/Unix stubs for GDI functions */

/* Stub implementations for Linux - these functions are Windows GDI-specific */

PHP_FUNCTION(printer_set_option) {
  php_error_docref(NULL, E_WARNING,
                   "printer_set_option is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_get_option) {
  php_error_docref(NULL, E_WARNING,
                   "printer_get_option is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_create_dc) {
  php_error_docref(NULL, E_WARNING,
                   "printer_create_dc is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_delete_dc) {
  php_error_docref(NULL, E_WARNING,
                   "printer_delete_dc is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_start_doc) {
  php_error_docref(NULL, E_WARNING,
                   "printer_start_doc is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_end_doc) {
  php_error_docref(NULL, E_WARNING,
                   "printer_end_doc is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_start_page) {
  php_error_docref(NULL, E_WARNING,
                   "printer_start_page is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_end_page) {
  php_error_docref(NULL, E_WARNING,
                   "printer_end_page is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_create_pen) {
  php_error_docref(NULL, E_WARNING,
                   "printer_create_pen is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_delete_pen) {
  php_error_docref(NULL, E_WARNING,
                   "printer_delete_pen is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_select_pen) {
  php_error_docref(NULL, E_WARNING,
                   "printer_select_pen is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_create_brush) {
  php_error_docref(NULL, E_WARNING,
                   "printer_create_brush is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_delete_brush) {
  php_error_docref(NULL, E_WARNING,
                   "printer_delete_brush is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_select_brush) {
  php_error_docref(NULL, E_WARNING,
                   "printer_select_brush is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_create_font) {
  php_error_docref(NULL, E_WARNING,
                   "printer_create_font is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_delete_font) {
  php_error_docref(NULL, E_WARNING,
                   "printer_delete_font is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_select_font) {
  php_error_docref(NULL, E_WARNING,
                   "printer_select_font is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_logical_fontheight) {
  php_error_docref(
      NULL, E_WARNING,
      "printer_logical_fontheight is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_draw_roundrect) {
  php_error_docref(NULL, E_WARNING,
                   "printer_draw_roundrect is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_draw_rectangle) {
  php_error_docref(NULL, E_WARNING,
                   "printer_draw_rectangle is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_draw_elipse) {
  php_error_docref(NULL, E_WARNING,
                   "printer_draw_elipse is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_draw_text) {
  zval *args;
  int argc;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "*", &args, &argc) == FAILURE) {
    RETURN_THROWS();
  }

  php_error_docref(NULL, E_WARNING,
                   "printer_draw_text is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_draw_line) {
  zval *args;
  int argc;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "*", &args, &argc) == FAILURE) {
    RETURN_THROWS();
  }

  php_error_docref(NULL, E_WARNING,
                   "printer_draw_line is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_draw_chord) {
  zval *args;
  int argc;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "*", &args, &argc) == FAILURE) {
    RETURN_THROWS();
  }

  php_error_docref(NULL, E_WARNING,
                   "printer_draw_chord is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_draw_pie) {
  zval *args;
  int argc;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "*", &args, &argc) == FAILURE) {
    RETURN_THROWS();
  }

  php_error_docref(NULL, E_WARNING,
                   "printer_draw_pie is only available on Windows (GDI)");
  RETURN_FALSE;
}

PHP_FUNCTION(printer_draw_bmp) {
  zval *args;
  int argc;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "*", &args, &argc) == FAILURE) {
    RETURN_THROWS();
  }
  php_error_docref(NULL, E_WARNING,
                   "printer_draw_bmp is only available on Windows (GDI)");
  RETURN_FALSE;
}

#endif /* PHP_WIN32 */

/* {{{ proto void printer_abort(resource handle)
   Abort printing*/
PHP_FUNCTION(printer_abort) {
  zval *zres;
  printer *resource;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zres) == FAILURE) {
    RETURN_THROWS();
  }

  if ((resource = (printer *)zend_fetch_resource(
           Z_RES_P(zres), "Printer Handle", le_printer)) == NULL) {
    RETURN_THROWS();
  }

#ifdef PHP_WIN32
  AbortPrinter(resource->handle);
#else
#ifdef HAVE_CUPS
  if (resource->job_id > 0 && resource->dest && resource->dest->name) {
    cupsCancelJob(resource->dest->name, resource->job_id);
  }
#else
  php_error_docref(NULL, E_WARNING, "CUPS support not compiled in");
#endif
#endif
}
/* }}} */

#ifdef PHP_WIN32
char *get_default_printer(void) {
  PRINTER_INFO_2 *printer;
  DWORD need, received;
  char *printer_name = NULL, *strtok_buf = NULL, buffer[250];

  if (GetVersion() < 0x80000000) {
    GetProfileString("windows", "device", ",,,", buffer, 250);
    php_strtok_r(buffer, ",", &strtok_buf);
    printer_name = pestrdup(buffer, 1);
  } else {
    EnumPrinters(PRINTER_ENUM_DEFAULT, NULL, 2, NULL, 0, &need, &received);
    if (need > 0) {
      printer = (PRINTER_INFO_2 *)emalloc(need + 1);
      EnumPrinters(PRINTER_ENUM_DEFAULT, NULL, 2, (LPBYTE)printer, need, &need,
                   &received);
      printer_name = pestrdup(printer->pPrinterName, 1);
      efree(printer);
    }
  }

  return printer_name;
}

int hex2dec(char hex) {
  switch (hex) {
  case 'F':
  case 'f':
    return 15;
    break;
  case 'E':
  case 'e':
    return 14;
    break;
  case 'D':
  case 'd':
    return 13;
    break;
  case 'C':
  case 'c':
    return 12;
    break;
  case 'B':
  case 'b':
    return 11;
    break;
  case 'A':
  case 'a':
    return 10;
  default:
    return (int)hex;
  }
}

/* convert a hexadecimal number to the rgb colorref */
COLORREF hex_to_rgb(char *hex) {
  int r = 0, g = 0, b = 0;

  if (strlen(hex) < 6) {
    return RGB(0, 0, 0);
  } else {
    r = hex2dec(hex[0]) * 16 + hex2dec(hex[1]);
    g = hex2dec(hex[2]) * 16 + hex2dec(hex[3]);
    b = hex2dec(hex[4]) * 16 + hex2dec(hex[5]);
    return RGB(r, g, b);
  }
}

/* convert an rgb colorref to hex number */
char *rgb_to_hex(COLORREF rgb) {
  char *string = emalloc(sizeof(char) * 6);
  sprintf(string, "%02x%02x%02x", GetRValue(rgb), GetGValue(rgb),
          GetBValue(rgb));
  return string;
}

static void printer_close(zend_resource *resource) {
  printer *p = (printer *)resource->ptr;

  ClosePrinter(p->handle);
  if (p->info.lpszDocName) {
    efree((char *)p->info.lpszDocName);
  }
  if (p->info.lpszOutput) {
    efree((char *)p->info.lpszOutput);
  }
  if (p->info.lpszDatatype) {
    efree((char *)p->info.lpszDatatype);
  }
  if (p->pi2) {
    if (p->pi2->pDevMode) {
      efree((char *)p->pi2->pDevMode);
    }
    efree((char *)p->pi2);
  }
  efree(p);
}
static void object_close(zend_resource *resource) {
  HGDIOBJ p = (HGDIOBJ)resource->ptr;
  DeleteObject(p);
}

#else
/* Linux/Unix CUPS implementations */

char *get_default_printer(void) {
#ifdef HAVE_CUPS
  cups_dest_t *dests;
  int num_dests;
  char *default_name = NULL;
  int i;

  num_dests = cupsGetDests(&dests);
  if (num_dests < 0) {
    return NULL;
  }

  for (i = 0; i < num_dests; i++) {
    if (dests[i].is_default) {
      default_name = pestrdup(dests[i].name, 1);
      break;
    }
  }

  if (!default_name && num_dests > 0) {
    default_name = pestrdup(dests[0].name, 1);
  }

  cupsFreeDests(num_dests, dests);
  return default_name;
#else
  return NULL;
#endif
}

static void printer_close(zend_resource *resource) {
  printer *p = (printer *)resource->ptr;

  if (!p) {
    return;
  }

  /* The CUPS-related http field in struct printer is always NULL here,
   * because CUPS_HTTP_DEFAULT is used throughout this extension.
   * As a result, there is no http handle to close in this cleanup
   * function. */
  if (p->name) {
    efree(p->name);
  }
  if (p->title) {
    efree(p->title);
  }
  if (p->datatype) {
    efree(p->datatype);
  }
#ifdef HAVE_CUPS
  if (p->dest) {
    cupsFreeDests(1, p->dest);
  }
  if (p->options) {
    cupsFreeOptions(p->num_options, p->options);
  }
#endif
  efree(p);
}

static void object_close(zend_resource *resource) {
  /* On Linux, we don't have GDI objects to clean up */
  /* This is kept for API compatibility */
}

#endif /* PHP_WIN32 */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */