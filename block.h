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

using namespace std;

class block{
    vector< vector< vector<int> > > value_;
    vector< unsigned int >parent_;
    int threshold_;
    
    void init_parent_(){
        int parent_size = this->value_.size()*this->value_.size()*this->value_.size();
        this->parent_.clear();
        this->parent_.resize(parent_size);
#pragma omp parallel for
        for(int i=0;i<parent_size;++i){
            this->parent_[i] = (unsigned int)i;
        }
        return ;
    }

    unsigned int find_parent_(unsigned int index){
        vector<int> index_for_update;

        while( parent_[index] != index){
            index_for_update.push_back(index);
            index = parent_[index];
        }
        for(unsigned int i=0;i<index_for_update.size();++i){
            parent_[ index_for_update[i] ] = index;
        }

        return index;
    }

    bool union_parent_(unsigned int index_a, unsigned int index_b){
        
        int root_a = find_parent_(index_a);
        int root_b = find_parent_(index_b);
        if(root_a == root_b){
            return false;
        }

        parent_[root_a] = root_b;
        return true;
    }

public:
    
    block(){
        this->value_.clear();
        this->threshold_ = -1;
    }

    block(const char *address_raw, int size,
            int byte_voxel, int threshold = 17000){
        this->read_raw( address_raw, size, byte_voxel, threshold );
    }

    void read_raw(const char *address_raw, int size,
            int byte_voxel,  int threshold = 17000);

    void union_all6();

    void output_parent(const char* address_parent);
};

struct block_coordinate{
    int size_block;
    int number_block_side;

    int index_block;
    int index_remain;
    int x;
    int y;
    int z;
    long long int index_whole;

    block_coordinate(){

    }
    
    block_coordinate(int sb, int nbs){
        this->size_block = sb;
        this->number_block_side = nbs;
    }

    void convert_from(int input_x, int input_y, int input_z){
        this->x = input_x;
        this->y = input_y;
        this->z = input_z;

        int index_block_x = input_x / this->size_block;
        int index_block_y = input_y / this->size_block;
        int index_block_z = input_z / this->size_block;
        this->index_block = index_block_z * this->number_block_side * this->number_block_side +
            index_block_y * this->number_block_side +
            index_block_x;

        int remained_x = input_x - index_block_x * this->size_block;
        int remained_y = input_y - index_block_y * this->size_block;
        int remained_z = input_z - index_block_z * this->size_block;
        this->index_remain = remained_z * this->size_block * this->size_block +
                            remained_y * this->size_block +
                            remained_x;

        long long int new_size_block = (long long int)number_block_side * (long long int)size_block ;

        this->index_whole = (long long int)z * new_size_block * new_size_block + (long long int)y * new_size_block + (long long int)x;
    }
};

class multi_block{
    vector<int16_t*> mmap_blocks_;
    vector<struct stat> stat_blocks_;
    vector<long long int*> mmap_parents_;

    int size_block_;
    int number_block_side_;
    int size_parent_;

    string directory_blocks_;
    string directory_parent_;


public:

    multi_block(){
        this->mmap_blocks_.clear();
        this->mmap_parents_.clear();
        this->directory_blocks_.clear();
    }

    multi_block(const char* directory_blocks, int number_raw,
            int size, const char* directory_parent){
        this->directory_blocks_ = string(directory_blocks);
        this->directory_parent_ = string(directory_parent);
        this->mmap_blocks_.resize(number_raw, NULL);
        this->stat_blocks_.resize(number_raw);
        this->size_block_ = size;
        this->number_block_side_ = (int)(pow(number_raw, 1/3) + 0.5);

        char original_directory[200] = {0};
        getcwd(original_directory, 200);
        chdir(directory_blocks);

        //mapping raw blocks
        for(int i=0;i<number_raw;++i){
            char address_raw[200] = {0};
            int fd_raw = -1;

            sprintf(address_raw, "%d.raw", i);
            fd_raw = open(address_raw, O_RDONLY);
            fstat(fd_raw, &stat_blocks_[i]);
            
            this->mmap_blocks_[i] = (int16_t*)mmap(NULL, stat_blocks_[i].st_size, PROT_READ, MAP_SHARED, fd_raw, 0);
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
        for(int i=0;i<number_raw;++i){
            char address_parent[200] = {0};
            int fd_parent = -1;
            
            sprintf(address_parent, "%d.prt", i);
            //create the parent file with the currect file size
            fd_parent = open(address_parent, O_RDWR | O_CREAT, 0644);
            this->size_parent_ = sizeof(long long int) * size_block_ * size_block_ * size_block_ ;
            lseek(fd_parent, this->size_parent_+1, SEEK_SET);
            write(fd_parent, "", 1);
            lseek(fd_parent, 0, SEEK_SET);
            
            //mapping
            this->mmap_parents_[i] = (long long int*)mmap(NULL, this->size_parent_, PROT_WRITE | PROT_READ, MAP_SHARED, fd_parent, 0);

            close(fd_parent);
        }
        
        chdir(original_directory);
    }

    ~multi_block(){
        for(unsigned int i=0;i<this->mmap_blocks_.size();++i){
            munmap(this->mmap_blocks_[i], this->stat_blocks_[i].st_size);
            munmap(this->mmap_parents_[i], this->size_parent_);
        }
    }

    int value(int x, int y, int z){
        block_coordinate coor(this->size_block_, this->number_block_side_);
        coor.convert_from(x, y, z);

        return (int)this->mmap_blocks_[coor.index_block][coor.index_remain];
    }

    void init_parent(void){
        for(int x=0;x<size_block_*number_block_side_;++x){
            for(int y=0;y<size_block_*number_block_side_;++y){
                for(int z=0;z<size_block_*number_block_side_;++z){
                    block_coordinate coor(this->size_block_, this->number_block_side_);
                    coor.convert_from(x, y, z);
                    this->mmap_parents_[coor.index_block][coor.index_remain] = coor.index_whole;
                }
            }
        }
        return;
    }

    void union_all6(int threshold);
};

#endif
