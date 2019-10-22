# tmpManager
Similar to tmpWatch, used to clean files in the specified directory.

## Install
```
git clone https://github.com/Laoli0519/tmpManager.git
cd tmpManager && mkdir build && cd build && cmake .. && make
```

## Usage
```
tmpManager    version 0.3-alpha
Usage : 
	-h  --help              :  help information
	-d  --directory x       :  scan the specified path file
	-c  --ctime x           :  base on file build time [/second]
	-m  --mtime x           :  base on file change time [/second]
	-u  --atime x           :  base on file access time [/second]
	-a  --all               :  scan all file [default : all]
	-i  --nodirs            :  exclude dir , not scan dir
	-f  --force             :  force operate
	-q  --quite             :  report error message only 
	-t  --test              :  test module
	-x  --exclude-path x    :  exclude the specified path file
	-y: --file-type x       :  scan the specified file-type  [file-type:-dlsbcp]
	-o: --move-mod x        :  file only, not delete one
	-U: --exclude-user x    :  exclude the specified user file
	-e: --max-depth x       :  recursive maximum number of layers
```
#### console
```
$tmpManager -a -t -d /tmp -o /home/test/
```
