#include <cstdio>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "progressbar.h"
#include "statusbar.h"
#include <map>

//#define PREFIX_SETS "/Users/toosyou/ext/Research/neuron_data/raw_noth/"
#define PREFIX_SETS "sets/"
#define PREFIX_SETIPTS "set_ipts/"

using namespace std;

int main(void){
    
	const short ver=5, com=0;
	const int n=64, width=530, height=530, zSize=530, tSize=1;
	const short BPP=2, PT=0;
	const int gray=65535;
	
    map<uint16_t, uint16_t> new_index;
    new_index[0] = (uint16_t)0;
    uint16_t index_sofar = 1;
    int total_index = 0;

    mkdir(PREFIX_SETIPTS, 0755);

    progressbar *progress = progressbar_new("converting", n);

	for(int c=0;c<n;c++){

        fstream outFile;
        fstream inFile;

    	char out_name[100];
        char in_name[100];


		sprintf(out_name,"%s%d_17000.ipt",PREFIX_SETIPTS, c);
		outFile.open(out_name, fstream::out | std::ios::binary);
        if(outFile.is_open() == false){
            cerr << "ERROR OPENING " << out_name <<endl;
            continue;
        }
		
		char Magic[5]=".ipt";
		outFile.write((const char*) Magic, 4);
		outFile.write((const char*) &ver, 2);
		outFile.write((const char*) &com, 2);
		
		outFile.write((const char*) &width, 4);	
		outFile.write((const char*) &height, 4);
		outFile.write((const char*) &zSize, 4);
		outFile.write((const char*) &tSize, 4);
		outFile.write((const char*) &BPP, 2);
		outFile.write((const char*) &PT, 2);
		outFile.write((const char*) &gray, 4);
		
		char zero = 0;
		for(int i=32; i<4096; i++) outFile.write((const char*) &zero, 1);
		//new ip2 version
        //for(int i=28; i<80; i++) outFile.write((const char*) &zero, 1);
        
        sprintf(in_name, "%s%d.set",PREFIX_SETS, c);
        inFile.open(in_name, fstream::in | fstream::binary);
        if(inFile.is_open() == false){
            cerr << "ERROR OPENING " << in_name <<endl;
            continue;
        }

        int number_new_index = 0;
        char buffer[64];
        while(  inFile.read(buffer, BPP) ){
            uint16_t index_old = *(uint16_t*)buffer;
            uint16_t index_new = 0;
            map<uint16_t, uint16_t>::iterator it = new_index.find( index_old );
            if( it != new_index.end()){//find
                index_new = it->second ;
            }else{
                new_index[index_old] = index_sofar;
                index_new = index_sofar;
                index_sofar += 1001;
                number_new_index++;
                total_index++;
            }
            
            outFile.write( (const char*)&index_new, BPP);
        }
	
	    outFile.close();
        inFile.close();

        cout << c << " number_new_index_in_block= " << number_new_index <<endl;

        progressbar_inc(progress);
	}
    progressbar_finish(progress);
    
    cout << "total_number_index= " << total_index ;

    return 0;
}
