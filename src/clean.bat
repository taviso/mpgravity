@echo off

pushd

pushd compface
rmdir /S /Q Debug
rmdir /S /Q Release
rmdir /S /Q lib
popd

pushd hunspell-1.2.8\src\hunspell
rmdir /S /Q Debug
rmdir /S /Q Release
popd

pushd libwinface
rmdir /S /Q Debug
rmdir /S /Q Release
rmdir /S /Q lib
popd

pushd pcre
rmdir /S /Q Debug
rmdir /S /Q Release
popd

pushd png
rmdir /S /Q Debug
rmdir /S /Q Release
popd

pushd setup\Setup
rmdir /S /Q Debug
rmdir /S /Q Release
popd

pushd zlib
rmdir /S /Q Debug
rmdir /S /Q Release
popd

rmdir /S /Q Debug
rmdir /S /Q Release

del *.user /s /f
del *.ncb /s /f
del *.mak /s /f
del *.suo /s /f
del *.aps /s /f

del *.pch /s /f
del *.idb /s /f
del *.pdb /s /f
del *.tlb /s /f
del *.tlh /s /f
del *.tli /s /f
del *.exp /s /f
del *.bsc /s /f
del *.manifest /s /f
del *_i.c /s /f
del *._i.h /s /f
del *_p.c /s /f
del *.res /s /f
del *.dep /s /f
del *.asm /s /f

popd
