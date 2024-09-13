import redis from '../../lib/redis';

export default async function handler(req, res) {

    const { userId } = req.query;

    try {
        const orderIds = await redis.smembers(`AAPL:user:${userId}:orders`);
        const userOrders = await Promise.all(orderIds.map(async (orderId) => {
            const orderDetails = await redis.hgetall(`AAPL:orders:${orderId}`);
            return {
                orderId,
                ...orderDetails,
                price: parseFloat(orderDetails.price),
                quantity: parseInt(orderDetails.quantity),
            };
        }));
        console.log("Success :)")
        res.status(200).json(userOrders);
    } catch (error) {
        console.error("Error fetching user orders:", error);
        res.status(500).json({ error: 'Failed to fetch user orders' });
    }
}