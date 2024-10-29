Redis architecture

Purpose
AAPL:orders:* (hash - order_id: {user_id, side, price, quantity, timestamp, status}) -> To retrieve order details
user:user0001:* ({order_id1, order_id2, order_id3 ...}) -> To quickly update user about his orders 


Market order
35=D (New Order Single)
40=1 (Market)
54=1 or 2 (Side: Buy or Sell)
55=AAPL (Symbol)
38=100 (OrderQty)
59=1 (TimeInForce: Immediate or Cancel)

Limit order
35=D (New Order Single)
40=2 (Limit)
54=1 or 2 (Side: Buy or Sell)
55=AAPL (Symbol)
38=100 (OrderQty)
44=150.50 (Price)
59=0 (TimeInForce: Day)