include ../GNUmakefile.common

CPPFLAGS += /I ../zlib

all: png.lib

Sources=png.c pngerror.c pngget.c pngmem.c pngpread.c pngrio.c pngrtran.c \
    pngrutil.c pngset.c pngtrans.c pngwio.c pngwrite.c pngwtran.c pngwutil.c \
    pngread.c

png.lib: $(Sources:.c=.obj)
	$(AR) $(LFLAGS) /OUT:$@ $^

