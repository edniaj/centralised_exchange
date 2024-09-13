import dynamic from 'next/dynamic'

const Orderbook = dynamic(() => import('../components/Orderbook'), { ssr: false })

export default function Home() {
  return (
    <div>
      <h1>Orderbook Frontend</h1>
      <Orderbook />
    </div>
  );
}