import { useEffect } from 'react';
import { useRouter } from 'next/router';
import Orderbook from '../components/Orderbook';

export default function Home() {
  const router = useRouter();

  useEffect(() => {
    // Check if the user is authenticated (you may want to implement a more robust check)
    const isAuthenticated = localStorage.getItem('isAuthenticated');
    if (!isAuthenticated) {
      router.push('/login');
    }
  }, []);

  return (
    <div>
      <h1>Orderbook</h1>
      <Orderbook />
    </div>
  );
}
