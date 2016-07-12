#ifndef BLOCK_COORDINATE_H
#define BLOCK_COORDINATE_H

#include <cstdio>
#include <iostream>

using namespace std;

struct block_coordinate{
    int size_block;
    int number_block_side;
    long long int new_size_block;

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
        this->new_size_block = (long long int)number_block_side * (long long int)size_block;
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

        int remained_x = input_x % this->size_block;
        int remained_y = input_y % this->size_block;
        int remained_z = input_z % this->size_block;
        this->index_remain = remained_z * this->size_block * this->size_block +
                            remained_y * this->size_block +
                            remained_x;

        this->index_whole = (long long int)z * new_size_block * new_size_block + (long long int)y * new_size_block + (long long int)x;

        return;
    }
    
    void convert_from(long long int whole){
        int new_size_block2 = new_size_block * new_size_block;
        this->index_whole = whole;
        this->z = index_whole / (new_size_block2);
        this->y = (index_whole % new_size_block2)/ new_size_block;
        this->x = index_whole % new_size_block ;
        
        int index_block_x = x / this->size_block;
        int index_block_y = y / this->size_block;
        int index_block_z = z / this->size_block;
        this->index_block = index_block_z * this->number_block_side * this->number_block_side +
            index_block_y * this->number_block_side +
            index_block_x;

        int remained_x = x % this->size_block;
        int remained_y = y % this->size_block;
        int remained_z = z % this->size_block;
        this->index_remain = remained_z * this->size_block * this->size_block +
                            remained_y * this->size_block +
                            remained_x;

        return;
    }

    void convert_from(const int input_index_block,const int input_index_reamin){
        this->index_block = input_index_block;
        this->index_remain = input_index_reamin;

        int number_block_side2 = this->number_block_side * this->number_block_side;
        int size_block2 = this->size_block * this->size_block;
        
        int index_block_z = index_block / number_block_side2;
        int index_block_y = ( index_block % number_block_side2 ) / number_block_side;
        int index_block_x = index_block % number_block_side;
        
        int remained_z = index_remain / size_block2;
        int remained_y = (index_remain % size_block2) / size_block;
        int remained_x = index_remain % size_block;

        this->x = index_block_x * size_block + remained_x;
        this->y = index_block_y * size_block + remained_y;
        this->z = index_block_z * size_block + remained_z;

        this->index_whole = (long long int)z * new_size_block * new_size_block + (long long int)y * new_size_block + (long long int)x;

    }

    bool operator==(block_coordinate &b){
        if( this->size_block == b.size_block &
                this->number_block_side == b.number_block_side &
                this->index_whole == b.index_whole)
            return true;
        return false;
    }

     bool operator!=(block_coordinate &b){
        return !this->operator==(b);
    }

};


#endif
