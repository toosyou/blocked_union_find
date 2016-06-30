#include <cstdio>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "progressbar.h"
#include "statusbar.h"

//#define PREFIX_SETS "/Users/toosyou/ext/Research/neuron_data/raw_noth/"
#define PREFIX_SETS "sets/"
#define PREFIX_SETIPTS "set_ipts/"

using namespace std;

int main(void){
    
	const short ver=5, com=0;
	const int n=64, width=530, height=530, zSize=530, tSize=1;
	const short BPP=2, PT=0;
	const int gray=65535;
	
    mkdir(PREFIX_SETIPTS, 0755);

    progressbar *progress = progressbar_new("converting", n);

#pragma omp parallel for
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

        char buffer[64];
        while(  inFile.read(buffer, BPP) ){
            outFile.write(buffer, BPP);
        }
	
	    outFile.close();
        inFile.close();

#pragma omp critical
        progressbar_inc(progress);
	}
    progressbar_finish(progress);

    return 0;
}
