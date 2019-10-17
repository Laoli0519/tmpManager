#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>
#include <errno.h>
#include <time.h>
#include "hTmpmanager.hpp"


tmpManager::tmpManager(const char* name) {
    // transferred_files_ = new std::vector<std::string*>;
    sub_dir_stack_ = new std::vector<const char*>;
    sub_dir_stack_->clear();
    logger_ = MiniLog::GetLog(name, MiniLog::log_level_debug);
}

tmpManager::~tmpManager() {
    // if(transferred_files_ != nullptr) {
    //     delete transferred_files_;
    // }
    
    if (sub_dir_stack_ != nullptr) {
        delete sub_dir_stack_;
        sub_dir_stack_ = nullptr;
    }
    if(scan_directory_ != nullptr) {
        delete scan_directory_;
        scan_directory_ = nullptr;
    }

    if(exclude_path_ != nullptr) {
        delete exclude_path_;
        exclude_path_ = nullptr;
    }

    if(file_type_ != nullptr) {
        delete file_type_;
        file_type_ = nullptr;
    }

    if(exclude_pattern_ != nullptr) {
        delete exclude_pattern_;
        exclude_path_ = nullptr;
    }
}

int tmpManager::run() {
    if(nullptr == scan_directory_) {
        logger_->error("Lack of necessary parameters");
        return -1;
    }

    int ret = 0;
    struct stat scan_dir_stat = {0};
    long int start_time = time(0);

    if(atime_ <= 0) {
        atime_ = start_time;
    }
    if(ctime_ <= 0) {
        ctime_ = start_time;
    }
    if(mtime_ <= 0) {
        mtime_ = start_time;
    }
    if(dir_mtime_ <= 0) {
        dir_mtime_ = start_time;
    }

    lstat(scan_directory_->c_str(), &scan_dir_stat);

    if(!S_ISDIR(scan_dir_stat.st_mode)) {
        logger_->error("The specified path is not a directory");
        return -2;
    }

    sub_dir_stack_->clear();
    sub_dir_stack_->emplace_back(scan_directory_->c_str());

    while(!sub_dir_stack_->empty()) {
        DIR *dir = NULL;
        struct dirent *dirptr = NULL;
        std::string basepath(sub_dir_stack_->back());

        sub_dir_stack_->pop_back();

        if ((dir = opendir(basepath.c_str())) == NULL) {
            logger_->error("Open {} error", basepath.c_str());
            continue;
        }

        //paichu zhiding wenjian leixing 
        //chdir()   "cd"
        while ((dirptr = readdir(dir)) != NULL) {
            memset(&scan_dir_stat, 0x00, sizeof(struct stat));
            std::string file_path = assemblyFullpath(basepath.c_str(), dirptr->d_name);

            lstat(file_path.c_str(), &scan_dir_stat);
            if (0 == strcmp(dirptr->d_name, ".") || 
                0 == strcmp(dirptr->d_name, "..") ||
                //TODO      dir_time
                scan_dir_stat.st_atim.tv_sec > start_time - atime_ ||
                scan_dir_stat.st_ctim.tv_sec > start_time - ctime_ ||
                scan_dir_stat.st_mtim.tv_sec > start_time - mtime_ ||
                0 != isExcludeFileType(dirptr->d_type) ||
                0 != isExcludeFileDir(file_path.c_str())) {   //current dir OR parrent dir
                continue;
            }
            
            if (dirptr->d_type == DT_DIR && nodirs_) {
                file_path.insert(file_path.end(), '/');
                if(max_depth_ >= raletiveLayerNum(scan_directory_->c_str(), file_path.c_str())) {   //dir
                    std::string mkdir_name = assemblyFullpath(move_directory_->c_str(), dirptr->d_name);
                    if(mkdir(mkdir_name.c_str(), 0755)) {
                        logger_->error("failed to created directory : {}, error : {}", mkdir_name.c_str(), strerror(errno));
                        continue;
                    }
                    sub_dir_stack_->emplace_back(file_path.c_str());
                }
            }
            else {
                handlewith(file_path.c_str(), dirptr->d_name);
            }
        }
        closedir(dir);
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
        exclude_path_ = new std::vector<const char*>;
    }

    std::string tmp(exclude_path);
    customPathFormat(tmp);
    exclude_path_->emplace_back(tmp.c_str());
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
    customPathFormat(*scan_directory_);
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
    customPathFormat(*move_directory_);
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
    
    // quite_ = true;
    logger_->setLogLevel(MiniLog::log_level_info);
    return 0;
}

void tmpManager::setTest() {
    test_ = true;
}

// TODO : fanhuizhi meiyou chuli
//file_path : full path (include file name)    /home/root/run.sh
//file_name : run.sh
inline int tmpManager::handlewith(const char* file_path, const char* file_name) {    
    int ret = 0;

    //Operate the file
    logger_->debug("Operate the {} file", file_path);
    if(nullptr != move_directory_) {
        std::string new_name = *move_directory_ + file_name;
        ret = rename(file_path, new_name.c_str());
        if (ret < 0) {
            logger_->error("Failed to move files : {} -> {}, error : {}", file_path, new_name.c_str(), strerror(errno));
            return -1;
        }
        logger_->info("Successfully moveing file : {} -> {}", file_path, new_name.c_str());
    } else {
        ret = remove(file_path);
        if (ret < 0) {
            logger_->error("Failed to delete files : {}, error : {}", file_path, strerror(errno));
            return -1;
        }
        logger_->info("Successfully deleted file : {}", file_path);
    }

    return 0;
}

inline int tmpManager::isExcludeFileType(const unsigned char d_type) {
    for(int i = 0; i < 7 && '\0' != file_type_[i]; ++i) {
        if(d_type == file_type_[i]) {
            return 1;
        }
    }
    return 0;
}

inline void tmpManager::customPathFormat(std::string& path) {
    if('/' != path[path.size-1]) {
        path.insert(path.end(), '/');
    }
}

inline int tmpManager::isExcludeFileDir(const char* file_dir) {
    for(auto i : *exclude_path_) {
        if(!strcmp(i, file_dir)) {
            return 1;
        }
    }

    return 0;
}

inline std::string assemblyFullpath(const char* base_path, const char* file_name) {
    std::string tmp(base_path);
    tmp.append(file_name);
    return  tmp;
}

//must be /home/   '/'
inline int raletiveLayerNum(const char* base_path, const char* file_path) {
    int len = strlen(base_path);
    int nTotal = 0;
	const char* str = file_path + len;

    while(nullptr != ( str = strchr(str, '/'))) {
        ++nTotal; 
        ++str;  
    }

    return nTotal + 1;
}