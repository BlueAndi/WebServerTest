#!/bin/bash

serverIpAddress="192.168.2.199"
testDataFilename="TEST300k.cam"
useCase=$1
numberOfTestRequests=$2
failCounter=0
successCounter=0

if [ -z "${useCase}" ]; then
    echo "Use case is missing."
    exit
fi

if [ -z "${numberOfTestRequests}" ]; then
    echo "Number of downloads is missing."
    exit
fi

for i in $(seq 1 ${numberOfTestRequests})
do
    response=$(curl -s --fail -w "%{time_total};%{speed_download}\n" -LO -X GET http://${serverIpAddress}/download/${useCase}/${testDataFilename});
    if [ 0 -eq $? ]; then

        IFS=';' read -ra ADDR <<< "${response}"
        totalTime=${ADDR[0]} # [s]
        downloadSpeed=$((ADDR[1] / 1024)) # [kb/s]

        echo "${i} Successful: ${totalTime}s ${downloadSpeed}kb/s"
        let successCounter++
    else
        echo "${i} Failed."
        let failCounter++
    fi;
done

echo "Successful downloads: ${successCounter} out of ${numberOfTestRequests}"
echo "Failed downloads    : ${failCounter} out of ${numberOfTestRequests}."
