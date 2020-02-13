#!/bin/bash
for i in {1..14}
do
	./build/client config/client.ini &
done

