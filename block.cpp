#include "block.h"

void block::read_raw(const char* address_raw, int size,
        int byte_voxel,  int threshold){
    
    //init value_
    this->value_.clear();
    this->value_.resize(size);
    for(int i=0;i<size;++i){
        this->value_[i].resize(size);
        for(int j=0;j<size;++j){
            this->value_[i][j].resize(size, 0);
        }
    }

    //read from raw data
    //mmap raw data first
    int fd_raw = open(address_raw, O_RDONLY);
    struct stat stat_raw;
    fstat(fd_raw, &stat_raw);
    char *mmap_raw = (char*)mmap(NULL, stat_raw.st_size, PROT_READ, MAP_SHARED, fd_raw, 0);
   
    //read from mmap
#pragma omp parallel for
    for(int i=0;i<size;++i){//z
        for(int j=0;j<size;++j){//y
            for(int k=0;k<size;++k){//x
                int offset = (i*size*size + j*size + k)*byte_voxel;
                int raw_value = 0;
                if(byte_voxel == 2){
                    raw_value = *((unsigned int16_t*)(mmap_raw+offset));
                }
                this->value_[i][j][k] = raw_value >= threshold ? 1 : 0;
            }
        }
    }

    //close all
    munmap(mmap_raw, stat_raw.st_size);
    close(fd_raw);
    return;
}
