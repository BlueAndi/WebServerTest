#!/bin/bash

serverIpAddress="192.168.2.199"
testDataFilename="TEST300k.cam"
numberOfTestRequests=400
failCounter=0

for i in $(seq 1 ${numberOfTestRequests})
do
    response=$(curl -s --fail -w "%{time_total}s\n" -LO -X GET http://${serverIpAddress}/testData/${testDataFilename});
    if [ 0 -eq $? ]; then
        echo "${i} Successful: ${response}"
    else
        echo "${i} Failed."
        let failCounter++
    fi;
done

echo "Failed ${failCounter} out of ${numberOfTestRequests}."
