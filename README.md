# centralised_exchange
g++ socket.cpp -o socket.exe | ./socket.exe

docker exec -it cpp_router /bin/bash

## todo
~~python sends a sample fix~~
~~gracefully exit server and client~~
~~ redis database - figure out how to set up ~~
~~Set up (mock data) and Delete all redis data in the database - python file~~
~~Create a frontend to view the orderbook~~
Fix disassembler to interpret the data - routes should be inside the disassembler
Set up database + mock user account
User authentication ->  Tag user to a FD in the router (work on authentication later)

Create routes to CRUD order into redis orderbook
Refactor the code
Create liquidation engine (?) 

implement edge-triggered over level-triggered
## Design decisions

Choosing level-triggered epoll over edge-triggered 
