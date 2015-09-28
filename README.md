# numcrunch
A collection of functions that crunches numbers

Currently implemented:
* clenshaw: both scalar and vectorized version

# compile
```sh
# cmake recommends out-of-source build, which keeps all the temp files in one place so you can wipe them out easily.
mkdir _build
cd _build
cmake ..
make
# executables can be found under _bin
# to try the binary using SIMD instructions:
_bin/clenshaw_vector -h
```
To generate assembly, which is handy if you want to verify certain optimization has kicked in, use the following:
```sh
SRC=
gcc -c -std=c11 -ggdb3 -O3 -fverbose-asm -S -march=native $SRC
```
