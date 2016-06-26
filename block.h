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

using namespace std;

class block{
    vector< vector< vector<int> > > value_;
    vector< unsigned int >parent_;
    
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
    }

    block(const char *address_raw, int size,
            int byte_voxel, int threshold = 17000){
        this->read_raw( address_raw, size, byte_voxel, threshold );
    }

    void read_raw(const char *address_raw, int size,
            int byte_voxel,  int threshold = 17000);

    void union_all6();
};

#endif
