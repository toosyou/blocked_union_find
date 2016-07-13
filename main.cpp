#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <dirent.h>

#include "block.h"

//#define PREFIX_RAW "/Users/toosyou/ext/Research/neuron_data/raw_noth/"
#define PREFIX_RAW "/Users/toosyou/ext/Research/neuron_data/GH146plus-M/reconstructed/raw64/"
#define PREFIX_PRTS "prts/"
#define PREFIX_SETS "sets/"

using namespace std;

int main(){

    multi_block blocks(PREFIX_RAW, 1, 433, 433, 261, PREFIX_PRTS, PREFIX_SETS);
    blocks.union_all6(17000);

    return 0;
}
