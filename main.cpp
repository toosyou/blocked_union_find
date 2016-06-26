#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>

#include "block.h"

#define PREFIX_RAW "/Users/toosyou/ext/Research/neuron_data/raw_noth/"

using namespace std;

int main(){
     
    for(int i=0;i<64;++i){
        char address_raw[200] = {0};
        char address_output_parent[200] = {0};
        sprintf(address_raw, "%s%d.raw", PREFIX_RAW, i);
        sprintf(address_output_parent, "%d.prt", i);

        cerr << "reading " << address_raw << " ...\t";
        cerr.flush();
        block this_block(address_raw, 530, 2);
        cerr << "done!" <<endl;
        
        this_block.union_all6();
        this_block.output_parent(address_output_parent);
    }

    return 0;
}
