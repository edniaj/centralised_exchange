# centralised_exchange
g++ socket.cpp -o socket.exe | ./socket.exe

docker exec -it cpp_router /bin/bash

## todo
~~python sends a sample fix~~
~~gracefully exit server and client~~
~~ redis database - figure out how to set up ~~
~~Set up (mock data) and Delete all redis data in the database - python file~~
~~Create a frontend to view the orderbook~~
~~Set up database + mock user account~~



Create efficient getter function to disassemble the fix protocol message
User authentication ->  Tag user to a FD in the router

Fix disassembler to interpret the data - routes should be inside the disassembler
Remove fd from verifiedfd once done


multi thread the logon with the orderbook updates

Create liquidation engine - redis is single thread os it can be handled in the routes. But what if we are using multiple redis db to handle the orderbook ?

Stream data instead of doing a get req (Redis) into the frontend

stream data using kafka message queue into frontend ? look into it

Copy FTX UI and finish up i guess


implement edge-triggered over level-triggered
## Design decisions

###Choosing level-triggered epoll over edge-triggered 
Potential busy loop when we user-authenticate while listening for orderbook updates
Solve - Multi-thread the authentication and orderbook updates. Authentication will assign file descriptor that orderbook will listen from