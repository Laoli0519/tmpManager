#include "hTmpmanager.hpp"



class mpManager {
private:

private:
    MiniLog::shared logger_ = nullptr;

    std::string *scan_directory = nullptr;
    std::string *exclude_path = nullptr;
    std::string *file_type = nullptr;
    std::string *exclude_pattern = nullptr;
    long ctime = 0;
    long atime = 0;
    long mtime = 0;
    long dir_mtime = 0;
    int max_depth = 0;

    bool nodirs = false;
    bool force = false;
    bool quite = false;
    bool test = false;
    bool verbose = false;
    bool move_module = false;
    bool all = false;
};


tmpManager::tmpManager(const char* name) {
    transferred_files_ = new std::vector<std::string*>;
    logger_ = MiniLog::GetLog(name, MiniLog::log_level_info);
}

tmpManager::~tmpManager() {
    if(transferred_files_ != nullptr) {
        delete transferred_files_;
    }

    if(scan_directory_ != nullptr) {
        delete scan_directory_;
    }

    if(exclude_path_ != nullptr) {
        delete exclude_path_;
    }

    if(file_type_ != nullptr) {
        delete file_type_;
    }

    if(exclude_pattern_ != nullptr) {
        delete exclude_pattern_;
    }
}

int tmpManager::run() {
    if(0 >= ctime_ && 0 >= atime_ && 0 >= mtime_ && 0 >= dir_mtime_ && nullptr == scan_directory_) {
        logger_->error();
        return -1;
    }
}

void tmpManager::setAtime(const long atime) {
    atime_ = atime;
}

void tmpManager::setCtime(const long ctime) {
    ctime_ = ctime;
}

void tmpManager::setMtime(const long mtime) {
    mtime_ = mtime;
}

void tmpManager::setDirmtime(const long dir_mtime) {
    dir_mtime_ = dir_mtime_;
}

void tmpManager::setExcludepattern(const char* exclude_pattern) {
    if(exclude_pattern_ == nullptr) {
        exclude_pattern_ = new std::string;
    }

    exclude_pattern_->clear();
    exclude_pattern_->append(exclude_pattern);
}

void tmpManager::setExcludePath(const char* exclude_path) {
    if(exclude_path_ == nullptr) {
        exclude_path_ = new std::string;
    }

    exclude_path_->clear();
    exclude_path_->append(exclude_path);
}

void tmpManager::setMaxdepth(const int max_depth) {
    max_depth_ = max_depth;
}

void tmpManager::setScanDirectory(const char* scan_directory) {
    if(scan_directory_ == nullptr) {
        scan_directory_ = new std::string;
    }

    scan_directory_->clear();
    scan_directory_->append(scan_directory);
}

void tmpManager::setFileType(const char* file_type) {
    if(file_type_ == nullptr) {
        file_type_ = new std::string;
    }

    file_type_->append(file_type);
}

void tmpManager::setForce() {
    force_ = true;
}

int tmpManager::setAll() {
    if(file_type_ != nullptr) {
        return -1;
    }

    all_ = true;
    return 0;
}

void tmpManager::setNodirs() {
    nodirs_ = true;
}

int tmpManager::setQuite() {
    if(true == all_) {
        return -1;
    }
    
    quite_ = true;
    return 0;
}

void tmpManager::setTest() {
    test_ = true;
}

// int tmpManager::setVerbose() {
//     if(true == quite_) {
//         return -1;
//     }

//     verbose_ = true;
//     return 0;
// }

void tmpManager::setMovemodule() {
    move_module_ = true;
}

