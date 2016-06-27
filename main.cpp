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

#pragma omp parallel for
    for(int i=0;i<64;++i){
        char address_raw[200] = {0};
        char address_output_parent[200] = {0};
        sprintf(address_raw, "%s%d.raw", PREFIX_RAW, i);
        sprintf(address_output_parent, "%s%d.prt", PREFIX_PRTS, i);

        block this_block(address_raw, 530, 2);
        
        this_block.union_all6();
        this_block.output_parent(address_output_parent);
        cerr << "union in block " << address_raw << " done!" <<endl;
    }

    return 0;
}
