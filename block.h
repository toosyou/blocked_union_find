#ifndef BLOCK_H
#define BLOCK_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include "progressbar.h"
#include "statusbar.h"
#include "block_coordinate.h"
#include <map>

using namespace std;


long long int index_parent(int size_block, int number_block_side, int index_block, int index_reamin);

template <class T>
class block_vector{

    vector<T> value_;

public:

    block_vector(){
        this->value_.clear();
    }

    void resize(int new_size, T init){
        this->value_.resize(new_size, init);
        return;
    }

    void clear(){
        this->value_.clear();
    }

    block_coordinate operator[](block_coordinate input_coor){
        block_coordinate result_coor(input_coor.size_block_x, input_coor.size_block_y, input_coor.size_block_z, input_coor.number_block_side);
        result_coor.convert_from( this->value_[input_coor.index_block][input_coor.index_remain] );
        return result_coor;
    }

    T& operator[](int index){
        return this->value_[index];
    }

};

class multi_block{
    vector<uint16_t*> mmap_blocks_;
    vector<struct stat> stat_blocks_;
    block_vector<long long int*> mmap_parents_;

    int size_block_x_;
    int size_block_y_;
    int size_block_z_;
    int number_block_side_;
    unsigned int size_parent_;

    string directory_blocks_;
    string directory_parent_;
    string directory_set_;

    block_coordinate find_parent_(int x, int y, int z){
        block_coordinate coor(this->size_block_x_, this->size_block_y_, this->size_block_z_, this->number_block_side_);
        coor.convert_from(x, y, z);
        vector<block_coordinate> optimising_list;

        while( this->mmap_parents_[coor] != coor ){
            optimising_list.push_back(coor);
            coor = this->mmap_parents_[coor];
        }

        //optimise
        for(unsigned int i=0;i<optimising_list.size();++i){
            block_coordinate &this_coor = optimising_list[i];
            this->mmap_parents_[this_coor.index_block][this_coor.index_remain] = coor.index_whole;
        }

        return coor;
    }

    bool union_parent_(int xa, int ya, int za, int xb, int yb, int zb){
        block_coordinate root_a = this->find_parent_(xa, ya, za);
        block_coordinate root_b = this->find_parent_(xb, yb, zb);
        if( root_a == root_b )//no need to union
            return false;

        this->mmap_parents_[root_b.index_block][root_b.index_remain] = root_a.index_whole;

        return true;
    }

    void append_xyz_(int index_block, int index_remain, FILE *file);
public:

    multi_block(){
        this->mmap_blocks_.clear();
        this->stat_blocks_.clear();
        this->mmap_parents_.clear();
        this->directory_blocks_.clear();
        this->directory_parent_.clear();
    }

    multi_block(const char* directory_blocks, int number_raw,
            int size_x, int size_y, int size_z, const char* directory_parent, const char* directory_set){

        //init values
        this->directory_blocks_ = string(directory_blocks);
        this->directory_parent_ = string(directory_parent);
        this->directory_set_ = string(directory_set);

        this->mmap_blocks_.resize(number_raw, NULL);
        this->stat_blocks_.resize(number_raw);
        this->mmap_parents_.resize(number_raw, NULL);

        this->size_block_x_ = size_x;
        this->size_block_y_ = size_y;
        this->size_block_z_ = size_z;
        this->number_block_side_ = (int)(pow(number_raw, 1.0/3.0) + 0.5);

        //create directory
        mkdir(directory_parent, 0755);
        mkdir(directory_set, 0755);

        //remember the original working directory
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

        chdir(original_directory);
    }

    ~multi_block(){
        #pragma omp parallel for
        for(unsigned int i=0;i<this->mmap_blocks_.size();++i){
            munmap(this->mmap_blocks_[i], this->stat_blocks_[i].st_size);
            munmap(this->mmap_parents_[i], this->size_parent_);
        }
    }

    int value(int x, int y, int z){
        block_coordinate coor(this->size_block_x_, this->size_block_y_, this->size_block_z_, this->number_block_side_);
        coor.convert_from(x, y, z);

        return (int)this->mmap_blocks_[coor.index_block][coor.index_remain];
    }

    void write_parent(int x, int y, int z, long long int parent){
        block_coordinate coor(this->size_block_x_, this->size_block_y_, this->size_block_z_, this->number_block_side_);
        coor.convert_from(x, y, z);
        this->mmap_parents_[coor.index_block][coor.index_remain] = parent;
        return ;
    }

    void init_parent(void){

        int size_parent_block = this->size_block_x_ * this->size_block_y_ * this->size_block_z_;
        int number_block = this->number_block_side_ * this->number_block_side_ * this->number_block_side_;
        progressbar *progress = progressbar_new("Init parent", number_block);

        for(int index_block = 0;index_block < number_block; ++index_block){
            #pragma omp parallel for
            for(int index_remain = 0;index_remain < size_parent_block;++index_remain){
                block_coordinate coor(this->size_block_x_, this->size_block_y_, this->size_block_z_, this->number_block_side_);
                coor.convert_from(index_block, index_remain);
                this->mmap_parents_[index_block][index_remain] = coor.index_whole;
            }
            progressbar_inc(progress);
        }

        progressbar_finish(progress);
        return;
    }

    void reindex();

    void union_all6(int threshold=17000);
};

#endif
