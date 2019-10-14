#include <unistd.h>
#include <getopt.h>
#include "minilogger.h"
#include "hTmpmanager.hpp"

#define LJH_DEBUG 0


static const struct option long_options[] = {  
        {"help",  no_argument,      NULL, 'h'},                         //scan the specified path file
        {"directory",  required_argument,      NULL, 'd'},            //scan the specified path file
        {"ctime",  required_argument,      NULL, 'c'},                //base on file build time
        {"mtime",  required_argument,      NULL, 'm'},                //base on file change time
        {"atime",  required_argument,      NULL, 'u'},                //base on file access time
        {"dirmtime",  required_argument,      NULL, 'M'},             //base on dir change time
        {"all",  no_argument,      NULL, 'a'},             //scan all file
        {"nodirs",  no_argument,      NULL, 'i'},               //exclude dir , not scan dir
        {"force",  no_argument,      NULL, 'f'},                //force
        {"quite",  no_argument,      NULL, 'q'},                //report error message only 
        {"test",  no_argument,      NULL, 't'},                 //test
        {"verbose",  no_argument,      NULL, 'v'},              //report full message 
        {"exclude-path",  required_argument,      NULL, 'x'},         //exclude the specified path file
        {"exclude-pattern",  required_argument,      NULL, 'X'},      //exclude the specified pattern file
        {"file-type",  required_argument,      NULL, 'y'},            //scan the specified file-type
        {"move-mod",  no_argument,      NULL, 'o'},                   //move file only, not delete one
        {"max-depth",  required_argument,      NULL, 'e'},            //recursive maximum number of layers
        {NULL,     0,                NULL,  0}
};

void usage() {
    printf("Usage : \n");
    printf("\t-h  --help              :  help information\n");
    printf("\t-d  --directory=x       :  scan the specified path file\n");
    printf("\t-c  --ctime=x           :  base on file build time [/second]\n");
    printf("\t-m  --mtime=x           :  base on file change time [/second]\n");
    printf("\t-u  --atime=x           :  base on file access time [/second]\n");
    printf("\t-M  --dirmtime=x        :  base on dir change time [/second]\n");
    printf("\t-a  --all               :  scan all file [default : all]\n");
    printf("\t-i  --nodirs            :  exclude dir , not scan dir\n");
    printf("\t-f  --force             :  force operate\n");
    printf("\t-q  --quite             :  report error message only \n");
    printf("\t-t  --test              :  test module\n");
    printf("\t-v  --verbose           :  report full message \n");
    printf("\t-x  --exclude-path=x    :  exclude the specified path file\n");
    //printf("\t-X: --exclude-pattern=x :  exclude the specified pattern file [Regular expression]\n");
    printf("\t-y: --file-type=x       :  scan the specified file-type  [file-type:-|d|l|s|b|c|p]\n");
    printf("\t-o: --move-mod          :  file only, not delete one\n");
    printf("\t-e: --max-depth=x       :  recursive maximum number of layers\n\n");

}

int cmdParse(int argc, char* argv[], tmpManager *tmp_manager) {
    int ret = 0;
    int opt = 0;
    int option_index = 0;
    const char* short_options = "hd:c:m:u:M:aifqtvx:X:t:oe:";
    
    while((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1 && 0 == ret) {  
#if LJH_DEBUG
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
                break;
            case 'c':
                break;
            case 'm':
                break;
            case 'u':
                break;
            case 'M':
                break;
            case 'a':
                break;
            case 'i':
                break;
            case 'f':
                break;
            case 'q':
                break;
            case 't':
                break;
            case 'v':
                break;
            case 'x':
                break;
            case 'y':
                break;
            case 'o':
                break;
            case 'e':
                break;
            case 'X':
            default: 
                printf("Invalid parameter -- -%c\n", opt);
                ret = -1;
                break;
        }
    }  

    return ret;
}

int cmdRun(int argc, char* argv[]) {

    if(argc < 2) {
        usage();
        return 0;
    }

    tmpManager *tmp_manager = new tmpManager("tmpManager");
    cmdParse(argc, argv, tmp_manager);
    tmp_manager->run();
}

int main(int argc, char* argv[]) {

    cmdRun(argc, argv);

    return 0;
}
