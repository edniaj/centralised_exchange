# centralised_exchange
g++ socket.cpp -o socket.exe | ./socket.exe

docker exec -it cpp_router /bin/bash

## todo
~~python sends a sample fix~~
~~gracefully exit server and client~~
~~ redis database - figure out how to set up ~~
~~Set up (mock data) and Delete all redis data in the database - python file~~
~~Create a frontend to view the orderbook~~
create a fix disassembler to interpret the data
Create routes to add data into redis orderbook
set up database
create fix route 

implement edge-triggered over level-triggered
## Design decisions

Choosing level-triggered epoll over edge-triggered 
