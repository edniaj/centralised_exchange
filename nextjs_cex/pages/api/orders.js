import redis from '../../lib/redis';

export default async function handler(req, res) {
    try {
        const symbol = 'AAPL'; // You can make this dynamic later
        const buyOrders = await getOrders(symbol, 'buy');
        const sellOrders = await getOrders(symbol, 'sell');
        
        // console.log("Buy Orders:", JSON.stringify(buyOrders, null, 2));
        // console.log("Sell Orders:", JSON.stringify(sellOrders, null, 2));
        
        res.status(200).json({ buyOrders, sellOrders });
    } catch (error) {
        console.error("Error in handler:", error);
        res.status(500).json({ error: 'Failed to fetch orders' });
    }
}

async function getOrders(symbol, side) {
    // Get all keys matching the pattern
    const keys = await redis.keys(`${symbol}:${side}:*`);
    // console.log(`Keys for ${symbol}:${side}:`, keys);

    const orders = [];
    for (const key of keys) {
        if (key.endsWith(':info')) {
            const price = key.split(':')[2];
            const aggregateInfo = await redis.hgetall(key);
            // console.log(`Aggregate info for ${key}:`, aggregateInfo);

            const orderIds = await redis.zrange(`${symbol}:${side}:${price}`, 0, -1);
            // console.log(`Order IDs for ${symbol}:${side}:${price}:`, orderIds);

            orders.push({
                price: parseFloat(price),
                totalQuantity: parseInt(aggregateInfo.total_quantity) || 0,
                orderCount: parseInt(aggregateInfo.order_count) || 0,
                orderIds,
            });
        }
    }

    // Sort buy orders in descending order, sell orders in ascending order
    return side === 'buy' 
        ? orders.sort((a, b) => b.price - a.price)
        : orders.sort((a, b) => a.price - b.price);
}