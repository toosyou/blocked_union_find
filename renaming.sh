#!/bin/bash

cd ./set_sep;

mkdir tmp

n=0;
ls -S *.set| while read i; 
do
    mv $i tmp/$n.set ;
    n=$((n+1));
    echo $n;
done

mv tmp/* ./
rm -f tmp
