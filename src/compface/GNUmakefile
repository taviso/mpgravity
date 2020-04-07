include ../../GNUmakefile.common

all: compface.lib

compface.lib: arith.obj compface.obj compress.obj file.obj gen.obj uncompface.obj
	$(AR) $(LFLAGS) /OUT:$@ $^
