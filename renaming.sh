#!/bin/bash

#backup
#echo "backuping";
#mkdir -p sets_backup;
#find ./sets -name '*.set' -exec cp {} ./sets_backup \;
#if [ $? -ne 0 ]; then
#    exit -1
#else
#    echo "backup succeed";
#fi

#exit -1;

#take sets whose size is larger than 11 bytes
echo "take sets whose size is larger than 11 bytes to larger"
rm -rf larger
mkdir -p larger
find ./sets -name '*.set' -size +11c -exec cp {} larger/ \;
echo "finished"

exit 0

#renaming 
cd larger
mkdir -p tmp

n=0;
find . -name '*.set' | xargs -I [] sh -c 'mv [] tmp/$n.set; n=$((n+1)); echo $n';

mv tmp/* ./
rmdir tmp

exit 0
