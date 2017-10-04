Toolchain building procedure
---

1. Run following command to build the toolchain ready to be installed in root
   filesystem. All files will be installed in `arm-none-eabi/usr`
   directory.
```
$ ./toolchain-arm build
```
2. Go to `arm-none-eabi` directory and build a Debian package with
   following command:
```
fakeroot ./debian/rules binary
```
