#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>
#include <string.h>
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
        logger_->error("Lack of necessary parameters");
        return -1;
    }

    int ret = 0;
    struct stat scan_dir_stat;
    std::vector<const char*> *sub_dir_stack = new std::vector<const char*>;
    

    
    stat(scan_directory_->c_str(), &scan_dir_stat);

    if(!S_ISDIR(scan_dir_stat.st_mode)) {
        logger_->error("The specified path is not a directory");
        return -2;
    }

    sub_dir_stack->emplace_back(scan_directory_->c_str());

    do {
        DIR *dir = NULL;
        struct dirent *dirptr = NULL;
        std::string basepath(sub_dir_stack->back());

        sub_dir_stack->pop_back();

        if ((dir = opendir(basepath.c_str())) == NULL) {
            logger_->error("Open {} error", basepath.c_str());
            continue;
        }

        //paichu zhiding wenjian leixing 
        //
        while ((dirptr = readdir(dir)) != NULL) {
            if (strcmp(dirptr->d_name, ".") == 0 || strcmp(dirptr->d_name, "..") == 0 ||
                0 == isExcludeFileType(dirptr->d_type)) {   //current dir OR parrent dir
                continue;
            }

            else if (dirptr->d_type == DT_REG) {   //file
                handlewith(basepath.c_str(), dirptr->d_name);
            }

            else if (dirptr->d_type == 4)  {   //dir
                memset(base, '\0', sizeof(base));
                strcpy(base, basePath);
                strcat(base, "/");
                strcat(base, dirptr->d_name);
            }
        }
        closedir(dir);
    } while(!sub_dir_stack->empty());



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
        exclude_path_ = new std::vector<const char*>;
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
    
    if('/' != scan_directory[strlen(scan_directory)-1]) {
        *scan_directory_ += '/';
    }
}

void tmpManager::setFileType(const char* file_type) {
    if(file_type_ == nullptr) {
        file_type_ = new unsigned char[8];
    }

    memset(file_type_, 0x00, 8);

    for(int i = 0; i < 7 && '\0' != file_type[i]; ++i) {
        switch(file_type[i]) {
        case '-':
            file_type_[i] = DT_REG;
            break;
        case 'l':
        case 'L':
            file_type_[i] = DT_LNK;
            break;
        case 'd':
        case 'D':
            file_type_[i] = DT_DIR;
            break;
        case 's':
        case 'S':
            file_type_[i] = DT_SOCK;
            break;
        case 'b':
        case 'B':
            file_type_[i] = DT_BLK;
            break;
        case 'c':
        case 'C':
            file_type_[i] = DT_CHR;
            break;
        case 'p':
        case 'P':
            file_type_[i] = DT_FIFO;
            break;
        default:
            logger_->error("invalid input file type = %c", file_type[i]);
        }
    }
}

void tmpManager::setMoveDirectory(const char* move_directory) {
    if(move_directory_ == nullptr) {
        move_directory_ = new std::string;
    }

    move_directory_->clear();
    move_directory_->append(move_directory);

    if('/' != move_directory[strlen(move_directory)-1]) {
        *move_directory_ += '/';
    }
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

// TODO : fanhuizhi meiyou chuli
int tmpManager::handlewith(const char* file_path, const char* file_name) {    
    int ret = 0;
    std::string old_file(file_path);
    old_file.append(file_name);

    if(nullptr != move_directory_) {
        std::string new_name = *move_directory_ + file_name;
        ret = rename(old_file.c_str(), new_name.c_str());
    } else {
        ret = remove(old_file.c_str());
    }

}

inline int tmpManager::isExcludeFileType(const unsigned char d_type) {
    for(int i = 0; i < 7 && '\0' != file_type_[i]; ++i) {
        if(d_type == file_type_[i]) {
            return 1;
        }
    }
    return 0;
}