# speed-test-service
client and server for speed test

## Build
make

## Run client 
./build/client `<path-to-config>`   
Example:   
./build/client config/client.ini

  ### output
  [Upload] `x` MB/s (for t seconds)   
  [Download] `x` MB/s (for t seconds)   

## Run server
./build/server `<path-to-config>`   
Example:   
./build/server config/server.ini
  
  ### output
  [connect-time] remote-address   
  Example:   
  [2020-01-20 12:32:48] 10.24.52.80:8000
