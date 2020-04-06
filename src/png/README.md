# Libpng

This is libpng 1.6.37, these are the steps I took to upgrade it


```
$ cmake.exe -A Win32 -S . -B build -D ZLIB_INCLUDE_DIR=../zlib -D ZLIB_LIBRARY=../zlib/zlib.lib
```

* Dump the required files from the vcxproj, remove everything else.

The required sources are something like this:

```
<ClCompile Include=...
```
