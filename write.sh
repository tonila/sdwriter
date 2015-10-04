#!/bin/bash
#echo $0
#echo $1
#echo $2

7zr x -so $1 | pv -ns $2 | dd of=$3 bs=512 conv=noerror,sync
#7zr x -so $1 | pv -ns $2
