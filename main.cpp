#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <dirent.h>
#include <fstream>
#include <unistd.h>

#include "block.h"

using namespace std;

int main(int argc, char* argv[]){

    if(argc == 1){
        cerr << "usage: ./blocked_union [-r] configure_file_address" <<endl;
        cerr << "-r : only do reindex" <<endl;
        return -1;
    }

    bool only_reindex = false;

    char command = '\0';
    while( (command = getopt(argc, argv, "r")) != -1 ){
        switch (command) {
            case 'r':
                only_reindex = true;
                break;
        }
    }

    //read configure file
    block_config configure(argv[optind]);

    multi_block blocks(configure);
    if(!only_reindex)
        blocks.union_all6(configure.threshold_background, configure.threshold_set_size);
    else
        blocks.reindex(configure.threshold_set_size);

    return 0;
}
