include ../GNUmakefile.common

CPPFLAGS += /D HAVE_CONFIG_H

all: pcre.lib

Sources=pcre_byte_order.c pcre_exec.c pcre_maketables.c pcre_study.c \
    pcre_xclass.c pcre_chartables.c pcre_fullinfo.c pcre_newline.c \
    pcre_tables.c pcre_compile.c pcre_get.c pcre_ord2utf8.c pcre_ucd.c \
    pcre_config.c pcre_globals.c pcre_refcount.c pcre_valid_utf8.c \
    pcre_dfa_exec.c pcre_jit_compile.c pcre_string_utils.c pcre_version.c

pcre.lib: $(Sources:.c=.obj)
	$(AR) $(LFLAGS) /OUT:$@ $^
