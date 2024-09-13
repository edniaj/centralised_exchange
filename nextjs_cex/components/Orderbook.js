import { useState, useEffect } from 'react';
import styles from './Orderbook.module.css';

export default function Orderbook() {
    const userId = "user0999";

    return (
        <div className={styles.orderbook}>
            <h1 className={styles.title}>AAPL Orderbook</h1>
            <OrderbookContent />
            <UserOrderDetails userId={userId} />
        </div>
    );
}

function OrderbookContent() {
    const [data, setData] = useState(null);
    const [error, setError] = useState(null);

    useEffect(() => {
        fetch('/api/orders')
            .then(res => res.json())
            .then(setData)
            .catch(setError);
    }, []);

    if (error) return <div>Failed to load orderbook data</div>;
    if (!data) return <div>Loading orderbook...</div>;

    const limitedBuyOrders = data.buyOrders.slice(0, 15);
    const limitedSellOrders = data.sellOrders.slice(0, 15);

    const maxQuantity = Math.max(
        ...limitedBuyOrders.map(o => o.totalQuantity),
        ...limitedSellOrders.map(o => o.totalQuantity)
    );

    return (
        <div className={styles.orderbookContent}>
            <OrderSide orders={limitedBuyOrders} side="Buy" maxQuantity={maxQuantity} />
            <OrderSide orders={limitedSellOrders} side="Sell" maxQuantity={maxQuantity} />
        </div>
    );
}

function OrderSide({ orders, side, maxQuantity }) {
    const isAsk = side === "Sell";
    return (
        <div className={`${styles.orderSide} ${isAsk ? styles.askSide : styles.bidSide}`}>
            <h2>{side} Orders</h2>
            <table className={styles.orderTable}>
                <thead>
                    <tr>
                        <th>Price</th>
                        <th>Quantity</th>
                        <th>Total</th>
                        <th>Depth</th>
                    </tr>
                </thead>
                <tbody>
                    {orders.map((order) => (
                        <tr key={order.price}>
                            <td className={styles.price}>{order.price.toFixed(2)}</td>
                            <td className={styles.quantity}>{order.totalQuantity}</td>
                            <td className={styles.total}>{order.orderCount}</td>
                            <td className={styles.depthVisualization}>
                                <div 
                                    className={styles.depthBar}
                                    style={{
                                        width: `${(order.totalQuantity / maxQuantity) * 100}%`,
                                        marginLeft: isAsk ? 'auto' : '0',
                                    }}
                                />
                            </td>
                        </tr>
                    ))}
                </tbody>
            </table>
        </div>
    );
}

function UserOrderDetails({ userId }) {
    const [userOrders, setUserOrders] = useState(null);
    const [error, setError] = useState(null);
    useEffect(() => {
        fetch(`/api/userOrders?userId=${userId}`)
            .then(res => res.json())
            .then(setUserOrders)
            .catch(setError);
    }, [userId]);

    if (error) return <div>Failed to load user orders</div>;
    if (!userOrders) return <div>Loading user orders...</div>;

    return (
        <div className={styles.userOrderDetails}>
            <h2>Orders for User {userId}</h2>
            <table className={styles.orderTable}>
                <thead>
                    <tr>
                        <th>Order ID</th>
                        <th>Side</th>
                        <th>Price</th>
                        <th>Quantity</th>
                        <th>Status</th>
                    </tr>
                </thead>
                <tbody>
                    {userOrders.map((order) => (
                        <tr key={order.orderId}>
                            <td>{order.orderId}</td>
                            <td>{order.side}</td>
                            <td>{order.price.toFixed(2)}</td>
                            <td>{order.quantity}</td>
                            <td>{order.status}</td>
                        </tr>
                    ))}
                </tbody>
            </table>
        </div>
    );
}   