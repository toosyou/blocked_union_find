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

void block::union_all6(){

    int size_value = this->value_.size();

    this->init_parent_();

    for(int i=0;i<size_value;++i){
        for(int j=0;j<size_value;++j){
            for(int k=0;k<size_value;++k){

                //there's no union with 0
                if(this->value_[i][j][k] == 0)
                    continue;

                unsigned index_now = i*size_value*size_value +
                                        j*size_value + k;

                for(int ii=-1;ii<=1;++ii){
                    for(int jj=-1;jj<=1;++jj){
                        for(int kk=-1;kk<=1;++kk){
                            if( abs(ii)+abs(jj)+abs(kk) > 1 )
                                continue;
                            int new_z = i+ii;
                            int new_y = j+jj;
                            int new_x = k+kk;

                            //check bound
                            if( new_z < 0 || new_z >= size_value ||
                                    new_y < 0 || new_y >= size_value || 
                                    new_x < 0 || new_x >= size_value)
                                continue;
                            
                            int index_target = new_z*size_value*size_value +
                                            new_y*size_value + new_x;
                            
                            if( this->value_[new_z][new_y][new_x] == 1 ){
                                //union
                                this->union_parent_(index_now, index_target);
                            }

                        }
                    }
                }
            }
        }
    }

    return ;
}
