# speed-test-service
client and server for speed test

## build
make

## Run client 
./build/client `<path-to-config>`
Example:   
./build/client config/client.ini

### sample output
[Download] `x` MB/s (for t seconds)   
[Upload] `x` MB/s (for t seconds)

## Run server
./build/server `<path-to-config>`   
Example:   
./build/server config/server.ini
  
  ### output
  [connect-time] remote-address   
  Example:   
  [2020-01-20 12:32:48] 10.24.52.80:8000
