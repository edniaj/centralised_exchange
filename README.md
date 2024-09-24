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

Fix protocol upgrade Validation

implement edge-triggered over level-triggered


## After establishing benchmarking testcases

### 1. More Efficient Encoding format - reduce overhead == reduce latency ^_^ (maybe custom serializer -deserializer since varying field format, this makes it even faster ) , ++ points for self-made optimization

### 2. Robust session management (Advanced Message Queueing protocol for better scalability ? not sure )

### 3. Kernel bypass on the server side. Lower latency in processing (DPDK/ SOLARFLARE might be preferable) - instead of copying to the kernel space, we access it directly in the user space (important!)
Traditional Flow:

Data Arrival: When data arrives from the network (like from a switch), it goes to the Network Interface Card (NIC).
Kernel Processing: The Operating System (OS) kernel handles this data using its networking stack, which involves several steps and can introduce delays.
Application Access: Your application (e.g., your C++ server) then accesses this data from the kernel, often through system calls that add more latency.
With Kernel Bypass:

Direct Access: Kernel Bypass allows your application to access network data directly from the NIC, skipping the OS kernel's networking stack.
Benefits: This reduces the number of steps and delays, leading to lower latency and higher throughput.

### 4. Zero-copy eliminate unnecessary data copying between different parts of your system (like between memory and network buffers)
Traditional Flow: Data might be copied multiple times:
From the NIC to kernel buffers.
From kernel buffers to your application’s buffers.
With Zero-Copy:
Minimized Copies: Data moves directly where it needs to go, such as from the NIC straight to your application's memory.
Benefits: This further reduces latency and saves CPU resources because less time is spent copying data around.

#### Why Use Both Kernel bypass and Zero-copying
Further Latency Reduction: While Kernel Bypass removes delays from the OS layer, Zero-Copy optimizes the data handling within your application.
Enhanced Performance: Especially critical in environments like high-frequency trading, where every microsecond counts.

## Design decisions

###Choosing level-triggered epoll over edge-triggered 
Potential busy loop when we user-authenticate while listening for orderbook updates
Solve - Multi-thread the authentication and orderbook updates. Authentication will assign file descriptor that orderbook will listen from

Open source the project ^_^