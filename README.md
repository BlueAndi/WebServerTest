# WebServerTest
It will connect to a wifi network and setup up a webserver with test data.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)
[![Repo Status](https://www.repostatus.org/badges/latest/wip.svg)](https://www.repostatus.org/#wip)

## Preparation of test data
1. Copy your test data to ```/data/testData``` folder. See test data files in ```/scripts```.

## Preparation of target software
1. Change the "board" in ```/platformio.ini``` to your used board.
2. Open ```/src/main.cpp``` and set
   1. ```WIFI_STATION_SSID``` to your wifi network SSID and
   2. ```WIFI_STATION_PASSWORD``` to your wifi network password.
3. Build and upload filesystem.
4. Build and upload software.
5. Connect to serial and monitor output.

## Preparation of test script
1. Open ```/scripts/test.sh``` and change:
   1. Set ```serverIpAddress``` to the test webserver ip-address, see console output.
   2. Set ```testDataFilename``` according to your choosen test data, see ```/data/testData``` folder.

## Download tests
1. Open git bash console.
2. Change to folder ```/scripts```
3. Run the script: ```$ ./test.sh <use-case> <number-of-downloads>```
   1. ```<use-case>```
      1. "uc1": Download in webserver context.
      2. "uc2": Download in main loop context.
   2. ```<number-of-downloads>```: Total number of consequtive downloads.

Example:
```bash
$ ./test.sh uc1 3
```

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/WebServerTest/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
