Redis architecture

Purpose
AAPL:buy:* (hash - Price_tick: {total_quantity, order_count})-> To quickly update frontend about Price Tick + Quantity
AAPL:sell:* (hash)-> To quickly update frontend about Price Tick + Quantity

AAPL:user:user0001:* (set) -> To quickly update user about his orders 

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