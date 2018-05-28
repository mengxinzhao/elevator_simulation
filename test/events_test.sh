#!/bin/sh

# a simple script to loop the test 1000 times to make sure no deadlock
count=0
while [ $count -lt 1000 ]
do
    ./events_test
    let count=count+1
done
