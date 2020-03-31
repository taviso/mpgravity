@echo off

pushd

rem Make copy of gravity build tree
rmdir /S /Q \gravity-source
mkdir \gravity-source
xcopy /S /C /I /F /H /Y \gravity3.0 \gravity-source

rem Delete bits we don't want

cd \gravity-source
rmdir /S /Q Debug
rmdir /S /Q Release
rmdir /S /Q CVS

cd \gravity-source\cabinet
rmdir /S /Q CVS

cd \gravity-source\compface
rmdir /S /Q CVS
rmdir /S /Q Debug
rmdir /S /Q Release

cd \gravity-source\hunspell-1.2.8
rmdir /S /Q CVS

cd \gravity-source\hunspell-1.2.8\src
rmdir /S /Q CVS

cd \gravity-source\hunspell-1.2.8\src\hunspell
rmdir /S /Q CVS
rmdir /S /Q Debug
rmdir /S /Q Release

cd \gravity-source\include
rmdir /S /Q CVS

cd \gravity-source\include\png
rmdir /S /Q CVS

cd \gravity-source\include\zlib
rmdir /S /Q CVS

cd \gravity-source\libwinface
rmdir /S /Q CVS
rmdir /S /Q Debug
rmdir /S /Q Release

cd \gravity-source\pcre
rmdir /S /Q CVS
rmdir /S /Q Debug
rmdir /S /Q Release

cd \gravity-source\png
rmdir /S /Q CVS
rmdir /S /Q Debug
rmdir /S /Q Release

cd \gravity-source\png\code
rmdir /S /Q CVS

cd \gravity-source\res
rmdir /S /Q CVS

cd \gravity-source\setup
rmdir /S /Q CVS
del *.exe /s /f

cd \gravity-source\setup\Setup
rmdir /S /Q CVS
rmdir /S /Q Debug
rmdir /S /Q Release

cd \gravity-source\zlib
rmdir /S /Q CVS
rmdir /S /Q Debug
rmdir /S /Q Release

cd \gravity-source\zlib\code
rmdir /S /Q CVS

cd \gravity-source

del *.user /s /f
del *.ncb /s /f
del *.mak /s /f
del *.suo /s /f
del *.aps /s /f

del *.pch /s /f
del *.idb /s /f
del *.pdb /s /f
del *.tlb /s /f
rem del *.tlh /s /f
rem del *.tli /s /f
rem del *.exp /s /f
rem del *.bsc /s /f
rem del *.manifest /s /f
rem del *_i.c /s /f
rem del *._i.h /s /f
rem del *_p.c /s /f
rem del *.res /s /f
rem del *.dep /s /f
rem del *.asm /s /f

popd
