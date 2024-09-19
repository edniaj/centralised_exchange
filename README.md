# centralised_exchange
g++ socket.cpp -o socket.exe | ./socket.exe

docker exec -it cpp_router /bin/bash

## todo
~~python sends a sample fix~~
~~gracefully exit server and client~~
~~ redis database - figure out how to set up ~~
~~Set up (mock data) and Delete all redis data in the database - python file~~
~~Create a frontend to view the orderbook~~

Set up database + mock user account

Fix disassembler to interpret the data - routes should be inside the disassembler



User authentication ->  Tag user to a FD in the router (work on authentication later)

Create liquidation engine - redis is single thread os it can be handled in the routes. But what if we are using multiple redis db to handle the orderbook ?

Stream data instead of doing a get req (Redis) into the frontend

stream data using kafka message queue into frontend ? look into it

Copy FTX UI and finish up i guess


implement edge-triggered over level-triggered
## Design decisions

Choosing level-triggered epoll over edge-triggered 
