## Implementation of ring buffer decision


### WITHOUT alignas (Problem):
Memory Layout: [producerCount|consumerCount] (same 64-byte cache line)

Scenario:
Time 1: Core 0 loads cache line
[Core 0 Cache]              [Core 1 Cache]
[prod:5|cons:3]            [prod:5|cons:3]

Time 2: Core 0 updates producerCount to 6
[Core 0 Cache]              [Core 1 Cache]
[prod:6|cons:3]            [prod:5|cons:3] <- STALE DATA!

Time 3: Cache coherency protocol invalidates Core 1's cache
[Core 0 Cache]              [Core 1 Cache]
[prod:6|cons:3]            [INVALIDATED]

Time 4: Core 1 must reload ENTIRE cache line just to read consumerCount

### WITH alignas(64) (Solution):
Memory Layout: 
[producerCount|padding][consumerCount|padding]
|--64 bytes--|        |--64 bytes--|

Scenario:
Time 1: Each core loads its own cache line
[Core 0 Cache]         [Core 1 Cache]
[prod:5|padding]       [cons:3|padding]

Time 2: Core 0 updates producerCount
[Core 0 Cache]         [Core 1 Cache]
[prod:6|padding]       [cons:3|padding] <- STILL VALID!

#### No need to reload cache line on Core 1!
TLDR - Cache lines are 64 Byte stored in the register. when we use multi-thread, the threads are assigned to different core as the OS sees fit. Thus if we struct it as 64 bytes for each consumer count, then the thread will never encounter
any cache incoherent issues since it doesnt share the cache line with other variable.


### challengs
Managing volatile memory and cache lines. 
