#!/bin/bash

for ((i = 1; i <= 100; i++))
do
    echo "./client execute 100 -u "sleep 5""

    ./client execute 100 -u "sleep 5"
done