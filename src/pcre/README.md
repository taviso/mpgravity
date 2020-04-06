# Pcre

This is pcre 8.44, these are the steps I took to upgrade it

* Downloaded `pcre-8.44.tar.gz`
* Use the `CMakeLists.txt` to generate a suitable configuration.


```
$ cmake.exe -A Win32 -S . -B build
```

* Dump the required files from pcre.vcxproj, remove everything else.

The required sources are something like this:

```
<ClCompile Include=...
```
