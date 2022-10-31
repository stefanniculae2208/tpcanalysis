#! /bin/bash
DIR=build/
if [ ! -d "$DIR" ]
then
    mkdir "$DIR"
fi
cd "$DIR"
cmake ../
make
cd ..