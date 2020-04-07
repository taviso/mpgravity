include GNUmakefile.common

all:
	+make -C src
	+make -C setup

install: all
	make -C setup install

uninstall:
	make -C setup uninstall

clean::
	make -C src clean
	make -C setup clean
