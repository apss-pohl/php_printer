dnl $Id$
dnl config.m4 for extension printer

PHP_ARG_ENABLE(printer, whether to enable printer support,
[  --enable-printer        Enable printer support])

if test "$PHP_PRINTER" != "no"; then
  dnl Optional CUPS install prefix, e.g. --with-cups=/opt/cups
  AC_ARG_WITH(cups,
  [  --with-cups[=DIR]      Set the CUPS installation prefix],
  [
    if test "$withval" != "no" -a "$withval" != "yes"; then
      CUPS_CONFIG="$withval/bin/cups-config"
    fi
  ])

  dnl Check for CUPS on Linux/Unix systems (not Windows)
  AC_MSG_CHECKING([for CUPS support])

  dnl Try to find cups-config (honor --with-cups if provided, then search common paths)
  if test -z "$CUPS_CONFIG" -o ! -x "$CUPS_CONFIG"; then
    AC_PATH_PROG(CUPS_CONFIG, cups-config, no, [/usr/local/bin:/usr/bin:/opt/cups/bin:$PATH])
  fi
  
  if test "$CUPS_CONFIG" != "no" -a -x "$CUPS_CONFIG"; then
    dnl Get CUPS compilation flags
    CUPS_CFLAGS=`$CUPS_CONFIG --cflags`
    CUPS_LIBS=`$CUPS_CONFIG --libs`
    
    PHP_EVAL_INCLINE($CUPS_CFLAGS)
    PHP_EVAL_LIBLINE($CUPS_LIBS, PRINTER_SHARED_LIBADD)
    
    AC_DEFINE(HAVE_CUPS, 1, [Whether you have CUPS support])
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no - CUPS not found, basic printing will not be available])
  fi
  
  AC_DEFINE(HAVE_PRINTER, 1, [Whether you have printer support])
  PHP_NEW_EXTENSION(printer, printer.c, $ext_shared)
  PHP_SUBST(PRINTER_SHARED_LIBADD)
fi
