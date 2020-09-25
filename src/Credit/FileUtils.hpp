#pragma once

#include <osdialog/osdialog.h>
#include <patch.hpp>
#include "../plugin.hpp"

namespace file_utils {

/** Retrieve the default directory and file name for saving */
void getDefaultFilePath (std::string& dir, std::string& filename) {
    auto path = APP->patch->path;
    if (path.empty()) {
    	dir = asset::user("patches");
    	system::createDirectory(dir);
    }
    else {
    	dir = string::directory(path);
    	filename = string::filename(path);
    }
}

/** Opens a dialog window to retrieve a file path to save to */
std::unique_ptr<char> getChosenFilePath() {
    std::string dir;
    std::string filename;
    getDefaultFilePath(dir, filename);

    std::unique_ptr<char> pathC;
    pathC.reset(osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), NULL));
        
    return pathC;
}

using FilePtr = std::unique_ptr<FILE, decltype(&std::fclose)>;

/** Returns a FilePtr to the file at the given path */
FilePtr getFilePtr(std::string pathStr) {
    if (string::filenameExtension(string::filename(pathStr)) == "") {
    	pathStr += ".txt";
    }

	return { std::fopen(pathStr.c_str(), "w"), &std::fclose };
}

} // file_utils
