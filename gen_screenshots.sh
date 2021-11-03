#!/bin/bash

if [[ "$OSTYPE" == "darwin"* ]]; then
    RACK_USER_DIR="$HOME/Documents/Rack2"
    RACK_EXE="/Applications/VCV Rack 2.app/Contents/MacOS/Rack"
elif [[ "$OSTYPE" == "msys" ]]; then
    RACK_USER_DIR="/c/Users/jatin/OneDrive/Documents/Rack2"
    RACK_EXE="/c/Program\ Files/VCV/Rack2/Rack.exe"
else
    echo "Script not implemented for this OS!"
    exit
fi

# delete existing screenshots
rm doc/Screenshots/*.png
rm $RACK_USER_DIR/screenshots/ChowDSP/*

# generate screenshots
"$RACK_EXE" -t 2

# copy new screenshots
cp $RACK_USER_DIR/screenshots/ChowDSP/*.png doc/Screenshots/
