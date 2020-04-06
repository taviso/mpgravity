# Zlib

This is zlib 1.2.11, these are the steps I took to upgrade it

* Downloaded `zlib-1.2.11.tar.gz`
* Use the `CMakeLists.txt` to generate a suitable `zconf.h`.


```
$ cmake.exe -A Win32 -S . -B build
```

* Dump the required files from zlib.vcxproj, remove everything else.

The required sources are something like this:

```
<ClCompile Include=...
```
