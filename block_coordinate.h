#ifndef BLOCK_COORDINATE_H
#define BLOCK_COORDINATE_H

#include <cstdio>
#include <iostream>

using namespace std;

struct block_coordinate{
    int size_block_x;
    int size_block_y;
    int size_block_z;
    int number_block_side;
    long long int new_size_block_x;
    long long int new_size_block_y;
    long long int new_size_block_z;

    int index_block;
    int index_remain;
    int x;
    int y;
    int z;
    long long int index_whole;
    int index_block_x;
    int index_block_y;
    int index_block_z;

    block_coordinate(){

    }

    block_coordinate(int sb_x, int sb_y, int sb_z, int nbs){
        this->size_block_x = sb_x;
        this->size_block_y = sb_y;
        this->size_block_z = sb_z;

        this->number_block_side = nbs;
        this->new_size_block_x = (long long int)number_block_side * (long long int)size_block_x;
        this->new_size_block_y = (long long int)number_block_side * (long long int)size_block_y;
        this->new_size_block_z = (long long int)number_block_side * (long long int)size_block_z;
    }

    void convert_from(const int input_x, const int input_y, const int input_z){
        this->x = input_x;
        this->y = input_y;
        this->z = input_z;

        this->index_block_x = input_x / this->size_block_x;
        this->index_block_y = input_y / this->size_block_y;
        this->index_block_z = input_z / this->size_block_z;
        this->index_block = index_block_z * this->number_block_side * this->number_block_side +
            index_block_y * this->number_block_side +
            index_block_x;

        int remained_x = input_x % this->size_block_x;
        int remained_y = input_y % this->size_block_y;
        int remained_z = input_z % this->size_block_z;
        this->index_remain = remained_z * this->size_block_x * this->size_block_y +
                            remained_y * this->size_block_x +
                            remained_x;

        this->index_whole = (long long int)z * new_size_block_x * new_size_block_y + (long long int)y * new_size_block_x + (long long int)x;

        return;
    }

    void convert_from(const long long int whole){
        int new_size_block2 = new_size_block_y * new_size_block_x;
        this->index_whole = whole;
        this->z = index_whole / (new_size_block2);
        this->y = (index_whole % new_size_block2)/ new_size_block_x;
        this->x = index_whole % new_size_block_x;

        this->index_block_x = x / this->size_block_x;
        this->index_block_y = y / this->size_block_y;
        this->index_block_z = z / this->size_block_z;
        this->index_block = index_block_z * this->number_block_side * this->number_block_side +
            index_block_y * this->number_block_side +
            index_block_x;

        int remained_x = x % this->size_block_x;
        int remained_y = y % this->size_block_y;
        int remained_z = z % this->size_block_z;
        this->index_remain = remained_z * this->size_block_x * this->size_block_y +
                            remained_y * this->size_block_x +
                            remained_x;

        return;
    }

    void convert_from(const int input_index_block,const int input_index_reamin){
        this->index_block = input_index_block;
        this->index_remain = input_index_reamin;

        int number_block_side2 = this->number_block_side * this->number_block_side;
        int size_block2 = this->size_block_x * this->size_block_y;

        this->index_block_z = index_block / number_block_side2;
        this->index_block_y = ( index_block % number_block_side2 ) / number_block_side;
        this->index_block_x = index_block % number_block_side;

        int remained_z = index_remain / size_block2;
        int remained_y = (index_remain % size_block2) / size_block_x;
        int remained_x = index_remain % size_block_x;

        this->x = index_block_x * size_block_x + remained_x;
        this->y = index_block_y * size_block_y + remained_y;
        this->z = index_block_z * size_block_z + remained_z;

        this->index_whole = (long long int)z * new_size_block_y * new_size_block_x + (long long int)y * new_size_block_x + (long long int)x;

    }

    void convert_from(const int ibx, const int iby, const int ibz, const int rmx, const int rmy, const int rmz){

        this->index_block_x = ibx;
        this->index_block_y = iby;
        this->index_block_z = ibz;

        this->index_block = index_block_z * this->number_block_side * this->number_block_side +
            index_block_y * this->number_block_side +
            index_block_x;

        this->index_remain = rmz * this->size_block_x * this->size_block_y +
                            rmy * this->size_block_x +
                            rmx;

        this->x = index_block_x * size_block_x + rmx;
        this->y = index_block_y * size_block_y + rmy;
        this->z = index_block_z * size_block_z + rmz;

        this->index_whole = (long long int)z * new_size_block_y * new_size_block_x + (long long int)y * new_size_block_x + (long long int)x;
        
    }

    bool operator==(block_coordinate &b){
        if( this->size_block_x == b.size_block_x &
            this->size_block_y == b.size_block_y &
            this->size_block_z == b.size_block_z &
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
