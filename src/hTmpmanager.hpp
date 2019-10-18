#ifndef __H_TMPMANAGER__
#define __H_TMPMANAGER__

#include <unistd.h>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>
#include <errno.h>
#include <time.h>
#include <string.h>
#include "minilogger.h"




class tmpManager {
public:
    tmpManager(const char* name);

    ~tmpManager();

    int run();
    
    void setScanDirectory(const char* scan_directory);
    
    void setExcludePath(const char* exclude_path);

    void setFileType(const char* file_type);

    // void setExcludepattern(const char* exclude_pattern);

    void setExcludeusers(const char* exclude_user);

    void setMoveDirectory(const char* move_directory);

    void setCtime(const long ctime);

    void setAtime(const long atime);

    void setMtime(const long mtime);

    // void setDirmtime(const long dir_mtime);

    void setMaxdepth(const int max_depth);

    void setNodirs();

    void setForce();

    int setQuite();

    void setTest();

    int setAll();

private:
    inline int handlewith(const char* file_path, const char* file_name);

    inline int isExcludeFileType(unsigned char d_type);

    inline std::string assemblyFullpath(const char* base_path, const char* file_name);

    inline int raletiveLayerNum(const char* base_path, const char* file_path);

    inline void customPathFormat(std::string& path);

    inline int isExcludeFileDir(const char* file_dir);

    inline void dump();
private:
    MiniLog::shared logger_ = nullptr;

    std::string *scan_directory_ = nullptr;
    std::vector<const char*> *exclude_path_ = nullptr;
    unsigned char* file_type_ = nullptr;
    // std::string *exclude_pattern_ = nullptr;
    std::string *move_directory_ = nullptr;
    std::vector<const char*> *exclude_uers_ = nullptr;
    std::vector<const char*> *sub_dir_stack_;

    long ctime_ = 0;
    long atime_ = 0;
    long mtime_ = 0;
    int max_depth_ = 0;

    bool nodirs_ = false;
    bool force_ = false;
    bool test_ = false;
    bool all_ = false;
};




tmpManager::tmpManager(const char* name) {
    // transferred_files_ = new std::vector<std::string*>;
    sub_dir_stack_ = new std::vector<const char*>;
    sub_dir_stack_->clear();
    logger_ = MiniLog::GetLog(name, MiniLog::log_level_info);
    logger_->info("begin scan file");
}

tmpManager::~tmpManager() {
    logger_->info("Scan the specified path end");
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

    if(exclude_uers_ != nullptr) {
        delete exclude_uers_;
        exclude_uers_ = nullptr;
    }

    if(file_type_ != nullptr) {
        delete file_type_;
        file_type_ = nullptr;
    }

    // if(exclude_pattern_ != nullptr) {
    //     delete exclude_pattern_;
    //     exclude_path_ = nullptr;
    // }

    if (move_directory_ != nullptr) {
        delete move_directory_;
        move_directory_ = nullptr;
    }
}

inline void tmpManager::dump() {
    if(scan_directory_)
        logger_->debug("scan_directory_ = [{}]", *scan_directory_);
    if(file_type_)
        logger_->debug("file_type_ = [{}]", file_type_);
    if(move_directory_)
        logger_->debug("move_directory_ = [{}]", *move_directory_);
    logger_->debug("ctime_ = [{}]", ctime_);
    logger_->debug("atime_ = [{}]", atime_);
    logger_->debug("mtime_ = [{}]", mtime_);
    logger_->debug("max_depth_ = [{}]", max_depth_);
    logger_->debug("nodirs_ = [{}]", nodirs_);
    logger_->debug("force_ = [{}]", force_);
    logger_->debug("test_ = [{}]", test_);
    logger_->debug("all_ = [{}]", all_);
    if(exclude_path_)
        for(auto i : *exclude_path_)
            logger_->debug("exclude_path_ = [{}]", i);
    if(exclude_uers_)
        for( auto i : *exclude_uers_)
            logger_->debug("exclude_uers_ = [{}]", i);
}

int tmpManager::run() {
    if(nullptr == scan_directory_) {
        logger_->error("Lack of necessary parameters : [{}]", "scan directory");
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

#ifdef DEBUG_DUMP
    logger_->setLogLevel(MiniLog::log_level_debug);
    dump();
#endif
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
            logger_->error("failed to open file : [{}], error : [{}]", basepath, strerror(errno));
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
                (!all_ && 
                (scan_dir_stat.st_atim.tv_sec > start_time - atime_ ||
                scan_dir_stat.st_ctim.tv_sec > start_time - ctime_ ||
                scan_dir_stat.st_mtim.tv_sec > start_time - mtime_ ||
                0 != isExcludeFileType(dirptr->d_type) ||
                0 != isExcludeFileDir(file_path.c_str()) ) ) ) {   //current dir OR parrent dir
                continue;
            }
            
            if (dirptr->d_type == DT_DIR && nodirs_) {
                customPathFormat(file_path);
                if(max_depth_ >= raletiveLayerNum(scan_directory_->c_str(), file_path.c_str())) {   //dir
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

// void tmpManager::setDirmtime(const long dir_mtime) {
//     dir_mtime_ = dir_mtime_;
// }

// void tmpManager::setExcludepattern(const char* exclude_pattern) {
//     if(exclude_pattern_ == nullptr) {
//         exclude_pattern_ = new std::string;
//     }

//     exclude_pattern_->clear();
//     exclude_pattern_->append(exclude_pattern);
// }

void tmpManager::setExcludePath(const char* exclude_path) {
    if(exclude_path_ == nullptr) {
        exclude_path_ = new std::vector<const char*>;
    }

    std::string tmp(exclude_path);
    customPathFormat(tmp);
    exclude_path_->emplace_back(tmp.c_str());
}

void tmpManager::setExcludeusers(const char* exclude_user) {
    if(exclude_uers_ == nullptr) {
        exclude_uers_ = new std::vector<const char*>;
    }

    exclude_uers_->emplace_back(exclude_user);
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
            logger_->error("invalid input file type = [{}]", file_type[i]);
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
    logger_->setLogLevel(MiniLog::log_level_error);
    return 0;
}

void tmpManager::setTest() {
    test_ = true;
}

// TODO : fanhuizhi meiyou chuli
//file_path : full path (include file name)    /home/root/run.sh
//file_name : run.sh
inline int tmpManager::handlewith(const char* file_path, const char* file_name) {    
    //Operate the file
    logger_->debug("Operate the file : [{}]", file_path);

    if(nullptr != move_directory_) {
        std::string new_file = *move_directory_ + file_name;
        if(!test_ && rename(file_path, new_file.c_str()) < 0) {
            logger_->error("Failed to move files : [{}] -> [{}], error : [{}]", file_path, new_file, strerror(errno));
            return -1;
        }
        logger_->info("Successfully moveing file : [{}] -> [{}]", file_path, new_file);
    } else {
        if(!test_ && remove(file_path) < 0) {
            logger_->error("Failed to delete files : [{}], error : [{}]", file_path, strerror(errno));
            return -1;
        }
        logger_->info("Successfully deleted file : [{}]", file_path);
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
    if('/' != path[path.size()-1]) {
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

inline std::string tmpManager::assemblyFullpath(const char* base_path, const char* file_name) {
    std::string tmp(base_path);
    tmp.append(file_name);
    return  tmp;
}

//must be /home/   '/'
inline int tmpManager::raletiveLayerNum(const char* base_path, const char* file_path) {
    int len = strlen(base_path);
    int nTotal = 0;
	const char* str = file_path + len;

    while(nullptr != ( str = strchr(str, '/'))) {
        ++nTotal; 
        ++str;  
    }

    return nTotal + 1;
}

#endif