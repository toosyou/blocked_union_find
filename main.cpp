#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <dirent.h>

#include "block.h"

#define PREFIX_RAW "/Users/toosyou/ext/Research/neuron_data/raw_noth/"
#define PREFIX_PRTS "prts/"
#define PREFIX_SETS "sets/"

using namespace std;

int main(){
   
    multi_block blocks(PREFIX_RAW, 64, 530, PREFIX_PRTS, PREFIX_SETS);
    blocks.union_all6(17000);

    return 0;
}
