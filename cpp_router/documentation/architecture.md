## CORRECT:

### 1. Connection Layer:
[FIX Clients] → [DNS/Load Balancer] → [Multiple FIX Gateways]
                                      (Each gateway handles max N connections)
#### Explaination
DNS Load Balancer will issue Fix Gateway IP address to the FIX Clients.
FIX Clients will send trade orders to Fix Gateway which will redirect the orders to the correct Symbol Router.

This architecture will allow us to horizontally scale the FIX Gateways so that MORE USERS can trade. 

Pain points: We do not want users to directly connect to the Matching Engine because the matching engine wants to resolve trades locally thus it is not ideal to horizontally scale the Matching Engine, it will make more sense to vertically scale the Matching Engine.


### 2. Order Processing:
[FIX Gateway] → [FIX GATEWAY :: Symbol Router] → [Matching Engine Instance]
                                  (Each symbol assigned to ONE specific instance)
#### Explaination
FIX Gateway will send the orders to the Symbol Router.
Symbol Router will then direct the orders to the correct Matching Engine Instance.



### 3. Persistence:
[Matching Engine] → [Redis Backup]
                    (Async backup, not for real-time matching)
#### Explaination
Trades should be resolved locally since we want to implement some algorithm for the matching engine. 
Redis Backup will backup the trades to the database so that we can use the historical data for the matching engine's algorithm.
We do not want to resolve the trade directly on redis because theres too much overhead + it will be very expensive to do it on AWS.

## INCORRECT (What we want to avoid):
[Client A] AAPL Order ----→ [Matching Engine 1] (Processing AAPL)
[ClientX B] AAPL Order ----→ [Matching Engine 2] (Also Processing AAPL) ❌ Race condition because Matching Engine doesn't know about other MEs and the trades are resolved locally.