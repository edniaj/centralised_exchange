import net from 'net';
import dotenv from 'dotenv';
import path from 'path';

// Load environment variables from ../dev.env
dotenv.config({ path: path.resolve(process.cwd(), '..', 'dev.env') });

export default async function handler(req, res) {
  if (req.method !== 'POST') {
    return res.status(405).json({ message: 'Method Not Allowed' });
  }

  const { username, password } = req.body;

  const client = new net.Socket();
  const HOST = process.env.TCP_HOST || 'localhost';
  const PORT = parseInt(process.env.TCP_PORT || '8888', 10);

  client.connect(PORT, HOST, () => {
    console.log('Connected to TCP server');

    // Construct FIX message for login
    const fixMessage = [
      '8=FIX.4.2',
      '35=A',
      `553=${username}`,
      `554=${password}`,
      '10=000'
    ].join('\x01') + '\x01';

    client.write(fixMessage);
  });

  client.on('data', (data) => {
    console.log('Received:', data.toString());
    client.destroy();

    // Parse the FIX message response
    const fixResponse = data.toString().split('\x01');
    const msgType = fixResponse.find(field => field.startsWith('35='))?.split('=')[1];

    if (msgType === 'A') {
      res.status(200).json({ message: 'Login successful' });
    } else {
      res.status(401).json({ message: 'Login failed' });
    }
  });

  client.on('close', () => {
    console.log('Connection closed');
  });

  client.on('error', (err) => {
    console.error('Connection error:', err);
    res.status(500).json({ message: 'Server error' });
  });
}
