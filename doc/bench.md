# Benchmarking

This repository also contains a benchmarking suite, for examining the performance of each module.

## Building

Building the benchmarks depends on the full VCV Rack source code. As such, you must first download the Rack source code, and be able to fully build Rack from scratch. For more information on this process see the [Rack documentation](https://vcvrack.com/manual/Building#building-rack).

Next, proceed to the Rack plugins folder, and build the ChowDSP-VCV plugin in this folder:
```bash
$ cd $RACK_DIR/plugins/
$ git clone https://github.com/jatinchowdhury18/ChowDSP-VCV.git
$ cd ChowDSP-VCV/
$ make
```

We are now ready to build and run the benchmarking suite.
```bash
$ make bench
$ ./bench.exe
```
