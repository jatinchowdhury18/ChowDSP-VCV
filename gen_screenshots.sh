#!/bin/bash

# delete existing screenshots
rm doc/Screenshots/*.png
rm /c/Users/jatin/OneDrive/Documents/Rack/screenshots/ChowDSP/*

# generate screenshots
/c/Program\ Files/VCV/Rack/Rack.exe -t 2

# copy new screenshots
cp /c/Users/jatin/OneDrive/Documents/Rack/screenshots/ChowDSP/*.png doc/Screenshots/
