#!/bin/bash

for ((i = 1; i <= 200; i++))
do
    echo "./client execute 100 -u "man ls""

    ./client execute 100 -u "man ls"
done