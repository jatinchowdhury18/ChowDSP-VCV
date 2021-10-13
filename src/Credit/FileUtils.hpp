#pragma once

#include <osdialog.h>
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
    	dir = system::getDirectory(path);
    	filename = system::getFilename(path);
    }
}

/** Opens a dialog window to retrieve a file path to save to */
std::unique_ptr<char> getChosenFilePath() {
    std::string dir;
    std::string filename;
    getDefaultFilePath(dir, filename);

    if(system::getExtension(filename) == ".vcv") {
        filename.erase(filename.end() - 4, filename.end());
        filename += ".txt";
    }

    std::unique_ptr<char> pathC;

    auto* filters = osdialog_filters_parse("Raw Text (.txt):txt,m;Markdown (.md):md");
    pathC.reset(osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), NULL));
    osdialog_filters_free(filters);
        
    return pathC;
}

using FilePtr = std::unique_ptr<FILE, decltype(&std::fclose)>;

/** Returns a FilePtr to the file at the given path */
FilePtr getFilePtr(std::string pathStr) {
    if(system::getExtension(pathStr) == "") {
    	pathStr += ".txt";
    }

	return { std::fopen(pathStr.c_str(), "w"), &std::fclose };
}

} // file_utils
