#include "block.h"

void block::read_raw(const char* address_raw, int size,
        int byte_voxel,  int threshold){
    
    //init value_
    this->threshold_ = threshold;
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
                
                //surrounding 6 voxels
                for(int ii=-1;ii<=1;++ii){
                    for(int jj=-1;jj<=1;++jj){
                        for(int kk=-1;kk<=1;++kk){
                            if( abs(ii)+abs(jj)+abs(kk) != 1 )
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
                }//surrounding 6 voxels
            }
        }
    }

    //find all again
    for(unsigned int i=0;i<this->parent_.size();++i){
        this->find_parent_(i);
    }

    return ;
}

void block::output_parent(const char* address_parent){
    
    FILE *file_parent = fopen(address_parent, "wb");

    fwrite( (char*)(&this->parent_[0]), 4, this->parent_.size(), file_parent );

    fclose(file_parent);
    return;
}

long long int index_parent(int size_block, int number_block_side, int index_block, int index_remain){

    long long int number_block_side2 = number_block_side * number_block_side;
    long long int size_block2 = size_block * size_block;

    int index_block_z = index_block / number_block_side2;
    int index_block_y = (index_block % number_block_side2) / number_block_side;
    int index_block_x = index_block % number_block_side;
    
    int z_block = index_remain / size_block2;
    int y_block = (index_remain % size_block2) / size_block ;
    int x_block = index_remain % size_block;

    int x = index_block_x * size_block + x_block;
    int y = index_block_y * size_block + y_block;
    int z = index_block_z * size_block + z_block;

    return ((long long int)z)*number_block_side2*size_block2 + ((long long int)y)*number_block_side*size_block + (long long int)x;
}


void multi_block::union_all6(int threshold){
    
    int total_size = this->size_block_ * this->number_block_side_;
    //init
    this->init_parent();

    progressbar *progress = progressbar_new("union all6", total_size);

    for(int z=0;z<total_size;++z){
        for(int y=0;y<total_size;++y){
            for(int x=0;x<total_size;++x){
                
                //it's below threshold wont be connected
                if(this->value(x, y, z) < threshold){
                    this->write_parent(x, y, z, 0ll);
                    continue;
                }

                //surrounding 6 points
                for(int dx=-1;dx<=1;++dx){
                    for(int dy=-1;dy<=1;++dy){
                        for(int dz=-1;dz<=1;++dz){

                            //only check surrounding 6 points
                            if( abs(dx) + abs(dy) + abs(dz) != 1 )
                                continue;
                            //check boundary
                            if( x+dx < 0 || x+dx >= total_size ||
                                    y+dy < 0 || y+dy >= total_size ||
                                    z+dz < 0 || z+dz >= total_size)
                                continue;

                            //threshold check
                            if( this->value(x+dx, y+dy, z+dz) < threshold )
                                continue;
                            
                            this->union_parent_(x, y, z, x+dx, y+dy, z+dz);

                        }
                    }
                }//surrounding 6 points
                
            }
        }
        progressbar_inc(progress);
    }
    progressbar_finish(progress);

    progress = progressbar_new("find all again", total_size);

    //find all again
    for(int x=0;x<total_size;++x){
        for(int y=0;y<total_size;++y){
            for(int z=0;z<total_size;++z){
                if(this->value(x, y, z) >= threshold)
                    this->find_parent_( x, y, z );
            }
        }
        progressbar_inc(progress);
    }
    progressbar_finish(progress);
    
    return ;
}
