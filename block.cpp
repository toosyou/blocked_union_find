#include "block.h"

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
    
    const int total_number_block = this->number_block_side_ * this->number_block_side_ * this->number_block_side_;
    const int total_remain = this->size_block_*this->size_block_*this->size_block_;
    const int total_size = this->size_block_ * this->number_block_side_;
    block_coordinate coor(this->size_block_, this->number_block_side_);

    //init
    this->init_parent();

    progressbar *progress = progressbar_new("union all6", total_number_block);

    for(int index_block=0;index_block<total_number_block;++index_block){
        for(int index_remain=0;index_remain<total_remain;++index_remain){
            coor.convert_from(index_block, index_remain);
            int x = coor.x;
            int y = coor.y;
            int z = coor.z;
 
            //it's below threshold wont be connected
            if(this->value(x, y, z) < threshold){
                this->write_parent(x, y, z, 0ll);
                continue;
            }
            //this->write_parent(x, y, z, (long long int)(x+y+z) );
            //continue;

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
        progressbar_inc(progress);
    }
    progressbar_finish(progress);

    progress = progressbar_new("find all again", total_number_block);

    //find all again
    for(int index_block=0; index_block < total_number_block; ++index_block){
        for(int index_remain=0; index_remain < total_remain; ++index_remain){
            coor.convert_from(index_block, index_remain);
            int x = coor.x;
            int y = coor.y;
            int z = coor.z;
            if(this->value(x, y, z) >= threshold)
                this->find_parent_( x, y, z );
        }
        progressbar_inc(progress);
    }
    progressbar_finish(progress);

    //reindexing to .set
    progress = progressbar_new("reindexing", total_number_block);
    map<long long int, uint16_t> new_index;
    uint16_t index_sofar = (uint16_t)1;
    new_index[0ll] = 0;

    for(int index_block=0; index_block < total_number_block; ++index_block){
        for(int index_remain=0; index_remain < total_remain; ++index_remain){

            //this->mmap_sets_[index_block][index_remain] = (uint16_t)this->mmap_parents_[index_block][index_remain];
            //continue;

            long long int old_index = this->mmap_parents_[index_block][index_remain];
            map<long long int, uint16_t>::iterator it = new_index.find(old_index);

            if( it != new_index.end() ){//find
                this->mmap_sets_[index_block][index_remain] = it->second;
            }else{//didnt find it
                new_index[old_index] = index_sofar;
                this->mmap_sets_[index_block][index_remain] = index_sofar;
                index_sofar++;
            }
        }
        progressbar_inc(progress);
    }
    progressbar_finish(progress);
    
    return ;
}
