#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <dirent.h>
#include <fstream>

#include "block.h"

using namespace std;

int main(int argc, char* argv[]){

    if(argc != 2){
        cerr << "usage: ./blocked_union configure_file_address" <<endl;
        return -1;
    }

    //read configure file
    block_config configure(argv[1]);

    multi_block blocks(configure);
    blocks.union_all6(configure.threshold_background, configure.threshold_set_size);

    return 0;
}
