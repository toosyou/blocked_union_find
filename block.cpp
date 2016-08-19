#include "block.h"

multi_block::multi_block(const char* directory_blocks, int number_raw,
            int size_x, int size_y, int size_z, const char* directory_parent, const char* directory_set){

    //init values
    this->directory_blocks_ = string(directory_blocks);
    this->directory_parent_ = string(directory_parent);
    this->directory_set_ = string(directory_set);

    this->mmap_blocks_.resize(number_raw, NULL);
    this->stat_blocks_.resize(number_raw);
    this->mmap_parents_.resize(number_raw, NULL);
    this->mmap_size_.resize(number_raw, NULL);

    this->size_block_x_ = size_x;
    this->size_block_y_ = size_y;
    this->size_block_z_ = size_z;
    this->number_block_side_ = (int)(pow(number_raw, 1.0/3.0) + 0.5);

    //create directory
    mkdir(directory_parent, 0755);
    mkdir(directory_set, 0755);

    //recording the original working directory
    char original_directory[200] = {0};
    getcwd(original_directory, 200);
    chdir(directory_blocks);

    //mapping raw blocks
    #pragma omp parallel for
    for(int i=0;i<number_raw;++i){
        char address_raw[200] = {0};
        int fd_raw = -1;

        sprintf(address_raw, "%d.raw", i);
        fd_raw = open(address_raw, O_RDONLY);
        fstat(fd_raw, &stat_blocks_[i]);

        this->mmap_blocks_[i] = (uint16_t*)mmap(NULL, stat_blocks_[i].st_size, PROT_READ, MAP_SHARED, fd_raw, 0);
        if(this->mmap_blocks_[i] == MAP_FAILED){
            cerr << i <<  " MMAP ERROR!" <<endl;
            close(fd_raw);
            exit(-1);
        }

        close(fd_raw);
    }
    chdir(original_directory);
    chdir(directory_parent);

    //mapping parent files
    this->size_parent_ = sizeof(long long int) * size_block_x_ * size_block_y_ * size_block_z_ ;
    #pragma omp parallel for
    for(int i=0;i<number_raw;++i){

        char address_parent[200] = {0};
        int fd_parent = -1;

        sprintf(address_parent, "%d.prt", i);

        //create the parent file with the currect file size
        fd_parent = open(address_parent, O_RDWR | O_CREAT, 0644);
        lseek(fd_parent, this->size_parent_+1, SEEK_SET);
        write(fd_parent, "", 1);
        lseek(fd_parent, 0, SEEK_SET);

        //mapping
        this->mmap_parents_[i] = (long long int*)mmap(NULL, this->size_parent_, PROT_WRITE | PROT_READ, MAP_SHARED, fd_parent, 0);
        if(this->mmap_parents_[i] == MAP_FAILED){
            cerr << i << "th PARENT MMAP ERROR!" <<endl;
            close(fd_parent);
            exit(-1);
        }

        close(fd_parent);
    }

    //mapping size files
    this->size_size_ = sizeof(unsigned int) * size_block_x_ * size_block_y_ * size_block_z_ ;
    #pragma omp parallel for
    for(int i=0;i<number_raw;++i){

        char address_size[200] = {0};
        int fd_size = -1;

        sprintf(address_size, "%d.size", i);

        //create the parent file with the currect file size
        fd_size = open(address_size, O_RDWR | O_CREAT, 0644);
        lseek(fd_size, this->size_size_+1, SEEK_SET);
        write(fd_size, "", 1);
        lseek(fd_size, 0, SEEK_SET);

        //mapping
        this->mmap_size_[i] = (unsigned int*)mmap(NULL, this->size_size_, PROT_WRITE | PROT_READ, MAP_SHARED, fd_size, 0);
        if(this->mmap_size_[i] == MAP_FAILED){
            cerr << i << "th SIZE MMAP ERROR!" <<endl;
            close(fd_size);
            exit(-1);
        }

        close(fd_size);
    }

    chdir(original_directory);
}

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

void multi_block::reindex(int threshold_size){
    const int total_remain = this->size_block_x_ * this->size_block_y_ * this->size_block_z_;
    const int total_blocks = this->number_block_side_ * this->number_block_side_ * this->number_block_side_;
    long long int index_sofar = 1;
    map<long long int, long long int> index_map; //index_prt to index_new
    cache::lru_cache<long long int, FILE*, file_expire> cache_file(150); //index_new to expriable_file
    map<long long int, long long int>::iterator it_index_map;
    block_coordinate coor(this->size_block_x_, this->size_block_y_, this->size_block_z_, this->number_block_side_);

    int cache_hit = 0;
    int cache_mis = 0;

    //init
    index_map[0ll] = 0ll; //background

    progressbar *progress = progressbar_new("reindexing", total_blocks);

    //change to the directory of sets
    char original_directory[100] = {0};
    getcwd(original_directory, 100);
    chdir(this->directory_set_.c_str());

    for(int index_block=0;index_block<total_blocks;++index_block){
        for(int index_remain=0;index_remain<total_remain;++index_remain){

            long long int value = this->mmap_parents_[index_block][index_remain];
            coor.convert_from(value);

            int size_set = this->mmap_size_[coor.index_block][coor.index_remain];

            //it's not bigger enoough or it's background
            if( value == 0 || size_set < threshold_size  )
                continue;

            it_index_map = index_map.find(value);

            if(it_index_map != index_map.end()){//find

                long long int index_new = it_index_map->second;
                //find expirable file in cache
                FILE* file = NULL;
                try{
                    file = cache_file.get(index_new); //in the cache just use it
                    cache_hit++;
                }catch(std::range_error re){// not in the cache, reopen

                    char address_new[100] = {0};
                    sprintf(address_new, "%lld.set", index_new);

                    file = fopen(address_new, "r+b");
                    cache_file.put(index_new, file);
                    cache_mis++;
                }
                this->append_xyz_(index_block, index_remain, file);

            }else{// not find

                long long int index_new = index_sofar++;
                index_map[value] = index_new;

                //data to output
                int new_size = 0;

                //create new file
                char address_new[100] = {0};
                sprintf(address_new, "%lld.set", index_new);

                FILE* file_new = fopen(address_new, "w+b");
                fwrite( &new_size, sizeof(int), 1, file_new );

                //write xyz
                this->append_xyz_(index_block, index_remain, file_new);

                //update FILE* to lru_cache
                cache_file.put(index_new, file_new);
            }
        }

        progressbar_inc(progress);
    }
    progressbar_finish(progress);
    chdir(original_directory);

    //cache_file will be closed with destructor and delete_struct in cache_file

    //output missing rate
    cout << "lru-cache missing rate: " << cache_mis << "/" << cache_hit+cache_mis << " = " << (double)cache_mis/(double)(cache_hit+cache_mis) <<endl;

    return;
}

void multi_block::find_all(void){
    const int total_number_block = this->number_block_side_ * this->number_block_side_ * this->number_block_side_;
    const int total_remain = this->size_block_x_ * this->size_block_y_ * this->size_block_z_;
    block_coordinate coor(this->size_block_x_, this->size_block_y_, this->size_block_z_, this->number_block_side_);
    progressbar *progress = progressbar_new("find all again", total_number_block);

    //find all again
    for(int index_block=0; index_block < total_number_block; ++index_block){
        for(int index_remain=0; index_remain < total_remain; ++index_remain){
            coor.convert_from(index_block, index_remain);
            int x = coor.x;
            int y = coor.y;
            int z = coor.z;
            if(this->mmap_parents_[index_block][index_remain] != 0)
                this->find_parent_( x, y, z );
        }
        progressbar_inc(progress);
    }
    progressbar_finish(progress);

    return;
}

void multi_block::union_block6(int threshold, int index_block){

    const int total_number_block = this->number_block_side_ * this->number_block_side_ * this->number_block_side_;
    const int total_remain = this->size_block_x_ * this->size_block_y_ * this->size_block_z_;
    const int total_size_x = this->size_block_x_ * this->number_block_side_;
    const int total_size_y = this->size_block_y_ * this->number_block_side_;
    const int total_size_z = this->size_block_z_ * this->number_block_side_;
    block_coordinate coor(this->size_block_x_, this->size_block_y_, this->size_block_z_, this->number_block_side_);

    for(int index_remain=0; index_remain<total_remain; ++index_remain){
        coor.convert_from(index_block, index_remain);
        int x = coor.x;
        int y = coor.y;
        int z = coor.z;

        //it's below threshold wont be connected
        if(this->value(x, y, z) < threshold){
            this->write_parent(x, y, z, 0ll);
            this->write_size(x, y, z, (unsigned int)0);
            continue;
        }

        //surrounding 6 points
        //which only checks the right, front and underneath of every point
        for(int dx=0;dx<=1;++dx){
            for(int dy=0;dy<=1;++dy){
                for(int dz=0;dz<=1;++dz){

                    //only check surrounding 6 points
                    if( abs(dx) + abs(dy) + abs(dz) != 1 )
                        continue;
                    //check boundary
                    if( x+dx < 0 || x+dx >= total_size_x ||
                            y+dy < 0 || y+dy >= total_size_y ||
                            z+dz < 0 || z+dz >= total_size_z)
                        continue;

                    //check index_block
                    coor.convert_from(x+dx, y+dy, z+dz);
                    if(coor.index_block != index_block) // out of block
                        continue;

                    //check threshold
                    if( this->value(x+dx, y+dy, z+dz) < threshold )
                        continue;

                    this->union_parent_(x, y, z, x+dx, y+dy, z+dz);
                }
            }
        }//surrounding 6 points
    }
    return;
}

void multi_block::union_all6(int threshold, int threshold_set_size){

    const int total_number_block = this->number_block_side_ * this->number_block_side_ * this->number_block_side_;
    const int total_remain = this->size_block_x_ * this->size_block_y_ * this->size_block_z_;
    const int total_size_x = this->size_block_x_ * this->number_block_side_;
    const int total_size_y = this->size_block_y_ * this->number_block_side_;
    const int total_size_z = this->size_block_z_ * this->number_block_side_;

    //init
    this->init_union_find();

    //union all blocks separately
    progressbar *progress = progressbar_new("union all blocks6", total_number_block);

    #pragma omp parallel for
    for(int index_block=0; index_block<total_number_block; ++index_block){
        this->union_block6(threshold, index_block);

        #pragma omp critical
        progressbar_inc(progress);
    }
    progressbar_finish(progress);

    //union between blocks
    progress = progressbar_new("union between blocks6", total_number_block);

    for(int index_block_x=0; index_block_x<number_block_side_; ++index_block_x){
        for(int index_block_y=0; index_block_y<number_block_side_; ++index_block_y){
            for(int index_block_z=0; index_block_z<number_block_side_; ++index_block_z){
                block_coordinate coor(this->size_block_x_, this->size_block_y_, this->size_block_z_, this->number_block_side_);

                //union the block on the right
                if(index_block_x != number_block_side_-1){
                    for(int rmy=0; rmy<size_block_y_; ++rmy){
                        for(int rmz=0; rmz<size_block_z_; ++rmz){
                            coor.convert_from( index_block_x, index_block_y, index_block_z, size_block_x_-1, rmy, rmz );
                            int x = coor.x;
                            int y = coor.y;
                            int z = coor.z;

                            if(this->value(x,y,z) >= threshold && this->value(x+1,y,z) >= threshold){
                                this->union_parent_(x, y, z, x+1, y, z);
                            }
                        }
                    }
                }

                //union the block on the front
                if(index_block_y != number_block_side_-1){
                    for(int rmx=0; rmx<size_block_x_; ++rmx){
                        for(int rmz=0; rmz<size_block_z_; ++rmz){
                            coor.convert_from( index_block_x, index_block_y, index_block_z, rmx, size_block_y_-1, rmz );
                            int x = coor.x;
                            int y = coor.y;
                            int z = coor.z;

                            if(this->value(x,y,z) >= threshold && this->value(x,y+1,z) >= threshold){
                                this->union_parent_(x, y, z, x, y+1, z);
                            }
                        }
                    }
                }

                //union the block on the underneath
                if(index_block_z != number_block_side_-1){
                    for(int rmx=0; rmx<size_block_x_; ++rmx){
                        for(int rmy=0; rmy<size_block_y_; ++rmy){
                            coor.convert_from( index_block_x, index_block_y, index_block_z, rmx, rmy, size_block_z_-1 );
                            int x = coor.x;
                            int y = coor.y;
                            int z = coor.z;

                            if(this->value(x,y,z) >= threshold && this->value(x,y,z+1) >= threshold){
                                this->union_parent_(x, y, z, x, y, z+1);
                            }
                        }
                    }
                }

                progressbar_inc(progress);
            }
        }
    }
    progressbar_finish(progress);

    //find all again
    this->find_all();

    //reindexing to .set
    this->reindex(threshold_set_size);

    return ;
}
