#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "block_coordinate.h"
#include "progressbar.h"
#include "statusbar.h"
#include <map>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>

#define PREFIX_PRTS "prts/"
#define PREFIX_SETS "set_sep/"

#define BLOCK_SIZE 530
#define NUMBER_BLOCK_SIDE 4

using namespace std;

void append_xyz(int index_block, int index_remain, FILE *file){
    //data to append
    block_coordinate coor(BLOCK_SIZE, NUMBER_BLOCK_SIDE);
    coor.convert_from(index_block, index_remain);
    uint16_t x = coor.x;
    uint16_t y = coor.y;
    uint16_t z = coor.z;

    //get number of points
    int number_points = 0;

    fseek(file, 0, SEEK_SET);
    fread(&number_points, sizeof(int), 1, file);
    
    //to the append position
    fseek(file, sizeof(int) + number_points*sizeof(uint16_t)*3, SEEK_SET);
    fwrite(&x, sizeof(uint16_t), 1, file);
    fwrite(&y, sizeof(uint16_t), 1, file);
    fwrite(&z, sizeof(uint16_t), 1, file);

    //currect the new size
    number_points++;
    fseek(file, 0, SEEK_SET);
    fwrite(&number_points, sizeof(int), 1, file);

    return;
}

int main(void){

    const int total_remain = 530*530*530;
    long long index_sofar = 1;
    map<long long int, long long int> index_map;
    map<long long int, string> index_file; //new index to file
    map<long long int, long long int>::iterator it_index_map;
    index_map[0ll] = 0;

    mkdir(PREFIX_SETS, 0755);

    progressbar *progress = progressbar_new("reindex", 64);

    for(int index_block=0;index_block<64;++index_block){
        
        char address_prt[100] = {0};
        sprintf(address_prt, "%s%d.prt", PREFIX_PRTS, index_block);
        FILE *file_prts = fopen(address_prt, "rb");

        for(int index_remain=0;index_remain<total_remain;++index_remain){
            long long int value = 0;
            fread( &value, sizeof(long long int), 1, file_prts );
            it_index_map = index_map.find(value);

            if(it_index_map != index_map.end()){//find
                if( it_index_map->second == 0 )
                    continue;

                string address_file = index_file[ it_index_map->second ];
                FILE *file = fopen(address_file.c_str(), "r+b");

                append_xyz(index_block, index_remain, file);

                fclose(file);

            }else{// not find
                index_map[value] = index_sofar++;

                //data to output
                int new_size = 0;
                
                //create new file
                char address_new[100] = {0};
                sprintf(address_new, "%s%lld.set", PREFIX_SETS, index_map[value]);

                FILE* file_new = fopen(address_new, "w+b");
                fwrite( &new_size, sizeof(int), 1, file_new );

                //write xyz
                append_xyz(index_block, index_remain, file_new);

                fclose(file_new);

                //update the file name
                index_file[ index_map[value] ] = string(address_new); 
            }
            
        }

        fclose(file_prts);

        progressbar_inc(progress);
    }
    progressbar_finish(progress);

    return 0;
}
