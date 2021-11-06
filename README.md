# ChowDSP for VCV

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Build Status](https://dev.azure.com/jatinchowdhury180362/ChowDSP-VCV/_apis/build/status/jatinchowdhury18.ChowDSP-VCV?branchName=master)](https://dev.azure.com/jatinchowdhury180362/ChowDSP-VCV/_build/latest?definitionId=3&branchName=master)

This repository contains [ChowDSP](https://ccrma.stanford.edu/~jatin/chowdsp) modules for [VCV Rack](https://vcvrack.com/). Official releases can be installed from the [VCV Rack Library](https://library.vcvrack.com/ChowDSP).

<img src="./doc/Screenshots/ChowTape.png" alt="Pic" height="300"><img src="./doc/Screenshots/ChowTapeComp.png" alt="Pic" height="300"><img src="./doc/Screenshots/ChowTapeLoss.png" alt="Pic" height="300"><img src="./doc/Screenshots/ChowTapeDegrade.png" alt="Pic" height="300"><img src="./doc/Screenshots/ChowTapeChew.png" alt="Pic" height="300"><img src="./doc/Screenshots/ChowPhaserFeedback.png" alt="Pic" height="300"><img src="./doc/Screenshots/ChowPhaserMod.png" alt="Pic" height="300"><img src="./doc/Screenshots/ChowFDN.png" alt="Pic" height="300"><img src="./doc/Screenshots/ChowRNN.png" alt="Pic" height="300"><img src="./doc/Screenshots/ChowModal.png" alt="Pic" height="300"><img src="./doc/Screenshots/ChowDer.png" alt="Pic" height="300"><img src="./doc/Screenshots/Werner.png" alt="Pic" height="300"><img src="./doc/Screenshots/ChowPulse.png" alt="Pic" height="300"><img src="./doc/Screenshots/Credit.png" alt="Pic" height="300"><img src="./doc/Screenshots/Warp.png" alt="Pic" height="300"><img src="./doc/Screenshots/ChowChorus.png" alt="Pic" height="300">

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

## Credits

ChowDSP VCV uses the [Eigen](http://eigen.tuxfamily.org/) and [r8lib](https://people.sc.fsu.edu/~jburkardt/f_src/r8lib/r8lib.html) libraries for linear algebra, as well as Stefano D'Angelo's [Wright Omega function approximations](http://www.dangelo.audio/dafx2019-omega.html). The ChowChorus effect is inspired by the "Ensemble" effect in the [Surge Synthesizer](https://surge-synthesizer.github.io/) and borrows their implementation for representing complex numbers with SIMD registers.

Module design assisted by [Margus Mets](mailto:hello@mmcreative.eu).

ChowDSP uses [Roboto fonts](https://github.com/googlefonts/roboto), licensed under the [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0).

## License

ChowDSP VCV Modules are licensed under the GPLv3 license. Enjoy!
