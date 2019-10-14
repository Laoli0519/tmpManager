#ifndef __H_TMPMANAGER__
#define __H_TMPMANAGER__

#include <unistd.h>
#include <string>
#include <vector>
#include "minilogger.h"


class tmpManager {
public:
    tmpManager(const char* name);

    ~tmpManager();

    int run();
    
    void setScanDirectory(const char* scan_directory);
    
    void setExcludePath(const char* exclude_path);

    void setFileType(const char* file_type);

    void setExcludepattern(const char* exclude_pattern);

    void setCtime(const long ctime);

    void setAtime(const long atime);

    void setMtime(const long mtime);

    void setDirmtime(const long dir_mtime);

    void setMaxdepth(const int max_depth);

    void setNodirs();

    void setForce();

    int setQuite();

    void setTest();

    // int setVerbose();

    void setMovemodule();

    int setAll();
private:

private:
    MiniLog::shared logger_ = nullptr;

    std::string *scan_directory_ = nullptr;
    std::string *exclude_path_ = nullptr;
    std::string *file_type_ = nullptr;
    std::string *exclude_pattern_ = nullptr;
    long ctime_ = 0;
    long atime_ = 0;
    long mtime_ = 0;
    long dir_mtime_ = 0;
    int max_depth_ = 0;

    bool nodirs_ = false;
    bool force_ = false;
    bool quite_ = false;
    bool test_ = false;
    // bool verbose_ = false;
    bool move_module_ = false;
    bool all_ = false;

    std::vector<std::string*> *transferred_files_;
};


#endif