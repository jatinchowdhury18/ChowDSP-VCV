# Changelog
All notable changes to this project will be documented in
this file.

## [Unreleased]
- Add 4x oversampling to Chow Tape.
- Refactor delay lines to use Lagrange interpolation instead of sinc
  interpolation. This improves performance and removes dependency on
  libsamplerate.

## [1.0.0] - 2019-03-10
- Initial release for ChowDSP VCV Rack modules. Modules include
  tape model, phaser feedback, phaser modulation, and feedback
  delay network reverb.