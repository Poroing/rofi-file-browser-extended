ACLOCAL_AMFLAGS=-I m4 -I submodules/libnkutils
plugindir=@rofi_PLUGIN_INSTALL_DIR@

noinst_LTLIBRARIES =
noinst_PROGRAMS=
EXTRA_DIST=
check_PROGRAMS=
TESTS=
include submodules/libnkutils/libnkutils.mk

plugin_LTLIBRARIES = filebrowser.la

filebrowser_la_SOURCES=\
        src/cmds.c\
        src/files.c\
        src/filebrowser.c\
        src/icons.c\
        src/keys.c\
        src/options.c\
        src/util.c

filebrowser_la_CFLAGS= @glib_CFLAGS@ @rofi_CFLAGS@ $(NKUTILS_CFLAGS) @gtk_CFLAGS@ @fb_CFLAGS@
filebrowser_la_LIBADD= @glib_LIBS@ @rofi_LIBS@ $(NKUTILS_LIBS) @gtk_LIBS@
filebrowser_la_LDFLAGS= -module -avoid-version
