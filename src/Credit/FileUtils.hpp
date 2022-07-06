#pragma once

#include <osdialog.h>
#include <patch.hpp>
#include "../plugin.hpp"

namespace file_utils {

/** Smart pointer alias for the char array returned by osdialog_file */
using osdialog_file_ptr = std::unique_ptr<char, decltype(&std::free)>;

/** Smart pointer alias for osdialog_filters */
using osdialog_filters_ptr = std::unique_ptr<osdialog_filters, decltype(&osdialog_filters_free)>;

/** Smart pointer alias for a C file object */
using FilePtr = std::unique_ptr<FILE, decltype(&std::fclose)>;

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

    if(system::getExtension(filename) == ".vcv") {
      filename.erase(filename.end() - 4, filename.end());
      filename += ".txt";
    }
}

/** Prompts the user to select a filepath to save to, and then performs the save action */
void saveToChosenFilePath (std::function<void(const char* path)>&& saveAction) {
    std::string dir;
    std::string filename;
    getDefaultFilePath(dir, filename);

#ifdef USING_CARDINAL_NOT_RACK
    // Cardinal currently requires us to use their async_dialog_filebrowser in place of
    // osdialog_file, since osdialog_file blocks the event loop.
    async_dialog_filebrowser(true, "credit.txt", dir.c_str(), "Save credit file", [saveAction](char* path) {
        saveAction(path);
        std::free(path);
    });
#else
    osdialog_filters_ptr filters { osdialog_filters_parse("Raw Text (.txt):txt,m;Markdown (.md):md"), osdialog_filters_free };
    osdialog_file_ptr path { osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), filters.get()), std::free };
    saveAction(path.get());
#endif
}

/** Returns a FilePtr to the file at the given path */
FilePtr getFilePtr(std::string pathStr) {
    if(system::getExtension(pathStr).empty()) {
    	pathStr += ".txt";
    }

    return { std::fopen(pathStr.c_str(), "w"), &std::fclose };
}

} // file_utils
