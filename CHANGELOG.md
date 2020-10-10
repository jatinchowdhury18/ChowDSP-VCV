# Changelog
All notable changes to this project will be documented in
this file.

## [Unreleased]
- Added variable oversampling options for ChowTape, ChowDer, Werner

## [1.3.1] - 2020-10-06
- Fixed Credit crashes on OSX
- Added attenuverters for Werner Filter
- Changed default save file for Credit

## [1.3.0] - 2020-09-27
- Added new filter based on Kurt Werner's SVF
- Added new pulse shaper module (ChowPulse)
- Added new module to generate patch credits
- Improved performance of modules with oversampling
- Improved performance of ChowDer tone stage

## [1.2.1] - 2020-09-12
- Fixed oversampling bug
- Fixed tape bug at low saturation levels
- Fixed IIR coefficient calculation for peaking filter
- Added manual links for individual modules
- Added publication links for Tape and Modal modules

## [1.2.0] - 2020-09-03
- New analog distortion module (ChowDer)
- Implement new GUI designs
- Improved performance for Tape module Newton-Raphson solver
- Improved performance for all IIR filters
- Improved documentation

## [1.1.0] - 2020-08-16
- New Recurrent Neural Network module.
- New modal filter module.
- Added 4x oversampling to Chow Tape.
- Refactored delay lines to use Lagrange interpolation instead of sinc
  interpolation. This improves performance and removes dependency on
  libsamplerate.

## [1.0.0] - 2019-07-30
- Initial release for ChowDSP VCV Rack modules. Modules include
  tape model, phaser feedback, phaser modulation, and feedback
  delay network reverb.