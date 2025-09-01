#!/bin/bash

clear

## GET #
# curl -v http://localhost:4221/

curl -v -X POST -H "Content-Type: application/json" -d '{"key":"value"}' http://localhost:4221/

# curl -v -X PUT -d "key=value" http://localhost:4221/

# curl -v -X DELETE http://localhost:4221/

# curl -v -X PATCH -d "key=newvalue" http://localhost:4221/

## HEAD #
# curl -v -I http://localhost:4221/
