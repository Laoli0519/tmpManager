#include <unistd.h>
#include <getopt.h>
#include <string.h>

#define DEBUG_DUMP 1

#include "minilogger.h"
#include "hTmpmanager.hpp"



static const struct option long_options[] = {  
        {"help",  no_argument,      NULL, 'h'},                         //scan the specified path file
        {"directory",  required_argument,      NULL, 'd'},            //scan the specified path file
        {"ctime",  required_argument,      NULL, 'c'},                //base on file build time
        {"mtime",  required_argument,      NULL, 'm'},                //base on file change time
        {"atime",  required_argument,      NULL, 'u'},                //base on file access time
        // {"dirmtime",  required_argument,      NULL, 'M'},             //base on dir change time
        {"all",  no_argument,      NULL, 'a'},             //scan all file
        {"nodirs",  no_argument,      NULL, 'i'},               //exclude dir , not scan dir
        // {"force",  no_argument,      NULL, 'f'},                //force
        {"quite",  no_argument,      NULL, 'q'},                //report error message only 
        {"test",  no_argument,      NULL, 't'},                 //test
        // {"verbose",  no_argument,      NULL, 'v'},              //report full message 
        {"exclude-path",  required_argument,      NULL, 'x'},         //exclude the specified path file
        // {"exclude-pattern",  required_argument,      NULL, 'X'},      //exclude the specified pattern file
        {"file-type",  required_argument,      NULL, 'y'},            //scan the specified file-type
        {"move-mod",  required_argument,      NULL, 'o'},                   //move file only, not delete one
        {"exclude-user",  required_argument,      NULL, 'U'},            //exclude the specified user file
        {"max-depth",  required_argument,      NULL, 'e'},            //recursive maximum number of layers
        {NULL,     0,                NULL,  0}
};

void usage() {
    printf("tmpManager    version 0.3-alpha\n");
    printf("Usage : \n");
    printf("\t-h  --help              :  help information\n");
    printf("\t-d  --directory x       :  scan the specified path file\n");
    printf("\t-c  --ctime x           :  base on file build time [1/second]\n");
    printf("\t-m  --mtime x           :  base on file change time [1/second]\n");
    printf("\t-u  --atime x           :  base on file access time [1/second]\n");
    // printf("\t-M  --dirmtime=x        :  base on dir change time [/second]\n");
    printf("\t-a  --all               :  scan all file [default : none]\n");
    printf("\t-i  --nodirs            :  exclude dir , not scan dir\n");
    // printf("\t-f  --force             :  force operate\n");
    printf("\t-q  --quite             :  report error message only [default : verbose]\n");
    printf("\t-t  --test              :  test module\n");
    // printf("\t-v  --verbose           :  report full message \n");
    printf("\t-x  --exclude-path x    :  exclude the specified path file\n");
    //printf("\t-X: --exclude-pattern=x :  exclude the specified pattern file [Regular expression]\n");
    printf("\t-y: --file-type x       :  scan the specified file-type  [file-type : -dlsbcp]\n");
    printf("\t-o: --move-mod x        :  file only, not delete one  [default : delete]\n");
    printf("\t-U: --exclude-user x    :  exclude the specified user file\n");
    printf("\t-e: --max-depth x       :  recursive maximum number of layers\n\n");

}

int cmdParse(int argc, char* argv[], tmpManager *tmp_manager) {
    int ret = 0;
    int opt = 0;
    int option_index = 0;
    const char* short_options = "hd:c:m:u:aiqtx:t:y:o:U:e:";
    
    while((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1 && 0 == ret) {  
#if DEBUG_DUMP 
        printf("opt = %c\t\t", opt);
        printf("optarg = %s\t\t",optarg);
        printf("optind = %d\t\t",optind);
        printf("argv[optind] =%s\t\t", argv[optind]);
        printf("option_index = %d\n",option_index);
#endif
        switch(opt) {
            case 'h': 
                usage();
                ret = -1;
                break;
            case 'd': 
                tmp_manager->setScanDirectory(optarg);
                break;
            case 'c': 
                {
                    long ctime = atol(optarg);
                    tmp_manager->setCtime(ctime);
                }
                break;
            case 'm':
                {
                    long mtime = atol(optarg);
                    tmp_manager->setMtime(mtime);
                }
                break;
            case 'u':
                {
                    long atime = atol(optarg);
                    tmp_manager->setAtime(atime);
                }
                break;
            // case 'M':
            //     {
            //         long dirmtime = atol(optarg);
            //         tmp_manager->setDirmtime(dirmtime);
            //     }
            //     break;
            case 'a':
                tmp_manager->setAll();
                break;
            case 'i':
                tmp_manager->setNodirs();
                break;
            // case 'f':
            //     tmp_manager->setForce();
            //     break;
            case 'q':
                tmp_manager->setQuite();
                break;
            case 't':
                tmp_manager->setTest();
                break;
            case 'x':
                tmp_manager->setExcludePath(optarg);
                break;
            case 'y':
                tmp_manager->setFileType(optarg);
                break;
            case 'o':
                tmp_manager->setMoveDirectory(optarg);
                break;
            case 'e':
                {
                    int max_depth = atoi(optarg);
                    tmp_manager->setMaxdepth(max_depth);
                }
                break;
            case 'U':
                tmp_manager->setExcludeusers(optarg);
            case 'X':
            default: 
                printf("Invalid parameter -- -%c\n", opt);
                ret = -1;
                break;
        }
    }  

    return ret;
}

int Run(int argc, char* argv[]) {
    int ret = 0;
    if(argc < 2) {
        usage();
        return 0;
    }

    tmpManager *tmp_manager = new tmpManager("tmpManager");
    ret = cmdParse(argc, argv, tmp_manager);
    if (ret < 0) {
        return -1;
    }
    ret = tmp_manager->run();

    return ret;
}

int main(int argc, char* argv[]) {

    Run(argc, argv);

    return 0;
}
