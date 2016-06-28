#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <dirent.h>

#include "block.h"

#define PREFIX_RAW "/Users/toosyou/ext/Research/neuron_data/raw_noth/"
#define PREFIX_PRTS "prts/"

using namespace std;

int main(){
   
    mkdir(PREFIX_PRTS, 0755);

    multi_block blocks(PREFIX_RAW, 1, 530, PREFIX_PRTS);
    blocks.union_all6(17000);

    return 0;
}
