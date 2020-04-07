include ../../GNUmakefile.common

CPPFLAGS += /D _WINDOWS

all: zlib.lib

Sources=adler32.c crc32.c gzclose.c gzread.c infback.c inflate.c trees.c \
        zutil.c compress.c deflate.c gzlib.c gzwrite.c inffast.c inftrees.c \
        uncompr.c

zlib.lib: $(Sources:.c=.obj)
	$(AR) $(LFLAGS) /OUT:$@ $^
