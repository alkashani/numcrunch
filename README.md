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
cd ../_bin
./clenshaw_vector -h
```
To generate assembly, which is handy if you want to verify certain optimization has kicked in, use the following:
```sh
SRC=
gcc -c -std=c11 -ggdb3 -O3 -fverbose-asm -S -march=native $SRC
```

# preliminary results
On my late 2013 MBP with 2.8 GHz Intel Core i7, here are some preliminary numbers. Full benchmarking coming soon.

```sh
$ ./clenshaw_scalar -d 100000 -p 10000 -r 10
total time:     27.355039 s
time per round: 2735.503900 ms
time per point: 273.550390 us

$ ./clenshaw_vector -d 100000 -p 10000 -r 10
total time:     7.184084 s
time per round: 718.408400 ms
time per point: 71.840840 us
```
