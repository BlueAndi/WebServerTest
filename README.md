# WebServerTest
It will connect to a wifi network and setup up a webserver with test data.

## Preparation of target software
1. Open ```/src/main.cpp``` and set
   1. ```WIFI_STATION_SSID``` to your wifi network SSID and
   2. ```WIFI_STATION_PASSWORD``` to your wifi network password.
2. Build and upload filesystem.
3. Build and upload software.
4. Connect to serial and monitor output.

## Preparation of test script
1. Open ```/scripts/test.sh``` and set
   1. ```serverIpAddress``` to the test webserver ip-address, see console output.

## Download tests
1. Open git bash console.
2. Change to folder ```/scripts```
3. Run the script: ```$ ./test.sh```
