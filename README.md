# ChowDSP for VCV

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Build Status](https://dev.azure.com/jatinchowdhury18/ChowDSP-VCV/_apis/build/status/jatinchowdhury18.ChowDSP-VCV?branchName=master)](https://dev.azure.com/jatinchowdhury18/ChowDSP-VCV/_build/latest?definitionId=1&branchName=master)

This repository contains [ChowDSP](https://ccrma.stanford.edu/~jatin/chowdsp) plugins built as VCV Rack modules.

To build these plugins, you must have the [VCV Rack development environment](https://vcvrack.com/manual/Building#building-rack-plugins) set up. Then use the following commands:
```bash
# clone repository
$ git clone https://github.com/jatinchowdhury18/ChowDSP-VCV.git

$ cd ChowDSP-VCV

$ make install
```

# Plugins

## CHOW Tape
CHOW Tape is a port of the [CHOW Tape Model](https://github.com/jatinchowdhury18/AnalogTapeModel) audio plugin. Has controls for tape bias, saturation and drive.

## CHOW Phaser
CHOW Phaser is a port of the [CHOW Phaser](https://github.com/jatinchowdhury18/ChowPhaser) audio plugin. It is recommended to use with a bipolar LFO as the "LFO" input.

## CHOW FDN
A feedback delay network reverb. You can use up to 16 delay lines, but note that more delay lines will use more CPU.

## License

ChowDSP VCV Modules are licensed under the GPLv3 license. Enjoy!
