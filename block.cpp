#include "block.h"

void multi_block::append_xyz_(int index_block, int index_remain, FILE *file){
    //data to append
    block_coordinate coor(this->size_block_x_, this->size_block_y_, this->size_block_z_, this->number_block_side_);
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

void multi_block::reindex(){
    const int total_remain = this->size_block_x_ * this->size_block_y_ * this->size_block_z_;
    const int total_blocks = this->number_block_side_ * this->number_block_side_ * this->number_block_side_;
    long long int index_sofar = 1;
    map<long long int, long long int> index_map;
    map<long long int, string> index_file; //new index to file
    map<long long int, long long int>::iterator it_index_map;
    index_map[0ll] = 0;

    progressbar *progress = progressbar_new("reindexing", total_blocks);

    //change to the directory of sets
    char original_directory[100] = {0};
    getcwd(original_directory, 100);
    chdir(this->directory_set_.c_str());

    for(int index_block=0;index_block<total_blocks;++index_block){
        for(int index_remain=0;index_remain<total_remain;++index_remain){

            long long int value = this->mmap_parents_[index_block][index_remain];
            it_index_map = index_map.find(value);

            if(it_index_map != index_map.end()){//find
                if( it_index_map->second == 0 )
                    continue;

                string address_file = index_file[ it_index_map->second ];
                FILE *file = fopen(address_file.c_str(), "r+b");

                this->append_xyz_(index_block, index_remain, file);

                fclose(file);

            }else{// not find
                index_map[value] = index_sofar++;

                //data to output
                int new_size = 0;

                //create new file
                char address_new[100] = {0};
                sprintf(address_new, "%lld.set", index_map[value]);

                FILE* file_new = fopen(address_new, "w+b");
                fwrite( &new_size, sizeof(int), 1, file_new );

                //write xyz
                this->append_xyz_(index_block, index_remain, file_new);

                fclose(file_new);

                //update the file name
                index_file[ index_map[value] ] = string(address_new);
            }
        }

        progressbar_inc(progress);
    }
    progressbar_finish(progress);
    chdir(original_directory);
    return;
}


void multi_block::union_all6(int threshold){

    const int total_number_block = this->number_block_side_ * this->number_block_side_ * this->number_block_side_;
    const int total_remain = this->size_block_x_ * this->size_block_y_ * this->size_block_z_;
    const int total_size_x = this->size_block_x_ * this->number_block_side_;
    const int total_size_y = this->size_block_y_ * this->number_block_side_;
    const int total_size_z = this->size_block_z_ * this->number_block_side_;
    block_coordinate coor(this->size_block_x_, this->size_block_y_, this->size_block_z_, this->number_block_side_);

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

            //surrounding 6 points
            for(int dx=-1;dx<=1;++dx){
                for(int dy=-1;dy<=1;++dy){
                    for(int dz=-1;dz<=1;++dz){

                        //only check surrounding 6 points
                        if( abs(dx) + abs(dy) + abs(dz) != 1 )
                            continue;
                        //check boundary
                        if( x+dx < 0 || x+dx >= total_size_x ||
                                y+dy < 0 || y+dy >= total_size_y ||
                                z+dz < 0 || z+dz >= total_size_z)
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
    this->reindex();

    return ;
}
