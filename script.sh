#!/bin/bash

for ((i = 1; i <= 100; i++))
do
    echo "./client execute 100 -u "man ls""
    echo "./client execute 2 -p "grep -v ˆ# /etc/passwd | cut -f7 -d: | uniq | wc -l""
    echo "./client execute 1000 -u "sleep 1""

    ./client execute 100 -u "man ls"
    ./client execute 2 -p "grep -v ˆ# /etc/passwd | cut -f7 -d: | uniq | wc -l"
    ./client execute 1000 -u "sleep 1"
done