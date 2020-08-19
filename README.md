# ChowDSP for VCV

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Build Status](https://dev.azure.com/jatinchowdhury18/ChowDSP-VCV/_apis/build/status/jatinchowdhury18.ChowDSP-VCV?branchName=master)](https://dev.azure.com/jatinchowdhury18/ChowDSP-VCV/_build/latest?definitionId=1&branchName=master)

This repository contains [ChowDSP](https://ccrma.stanford.edu/~jatin/chowdsp) plugins built as VCV Rack modules.

<img src="./doc/ChowTape_cap.PNG" alt="Pic" height="300">
<img src="./doc/ChowPhaseFeedback_cap.PNG" alt="Pic" height="300">
<img src="./doc/ChowPhaseMod_cap.PNG" alt="Pic" height="300">
<img src="./doc/ChowFDN_cap.PNG" alt="Pic" height="300">
<img src="./doc/ChowRNN_cap.PNG" alt="Pic" height="300">
<img src="./doc/ChowModal_cap.PNG" alt="Pic" height="300">

For more information, see the [User Manual](./doc/manual.md).

## Building
To build these plugins, you must have the [VCV Rack development environment](https://vcvrack.com/manual/Building#building-rack-plugins) set up. Then use the following commands:
```bash
# clone repository
$ git clone https://github.com/jatinchowdhury18/ChowDSP-VCV.git

$ cd ChowDSP-VCV

$ make install
```

The ChowDSP-VCV repository also contains a benchmarking suite. For more information on building and running benchmarks, see the [benchmark documentation](./doc/bench.md).

## License

ChowDSP VCV Modules are licensed under the GPLv3 license. Enjoy!
