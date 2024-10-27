import React, { useState, useEffect } from 'react';
import { createLogonMessage, parseFixMessage } from '../utils/fixUtils';

function Login({ onLogin }) {
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [socket, setSocket] = useState(null);

  useEffect(() => {
    // Create WebSocket connection
    const ws = new WebSocket('ws://localhost:8888');

    ws.onopen = () => {
      console.log('WebSocket connected');
      setSocket(ws);
    };

    ws.onmessage = (event) => {
      console.log('Received FIX message:', event.data);
      const parsedMessage = parseFixMessage(event.data);
      console.log('Parsed FIX message:', parsedMessage);

      if (parsedMessage['35'] === 'A') {
        console.log('Authentication successful');
        onLogin(true);
      } else {
        console.error('Authentication failed');
        // Handle login error (e.g., show error message)
      }
    };

    ws.onerror = (error) => {
      console.error('WebSocket error:', error);
    };

    ws.onclose = () => {
      console.log('WebSocket disconnected');
    };

    return () => {
      if (ws) {
        ws.close();
      }
    };
  }, [onLogin]);

  const handleSubmit = (e) => {
    e.preventDefault();
    if (socket) {
      const logonMessage = createLogonMessage(username, password);
      console.log('Sending Logon message:', logonMessage);
      socket.send(logonMessage);
    } else {
      console.error('WebSocket not connected');
    }
  };

  return (
    <div>
      <h2>Login</h2>
      <form onSubmit={handleSubmit}>
        <input
          type="text"
          value={username}
          onChange={(e) => setUsername(e.target.value)}
          placeholder="Username"
          required
        />
        <input
          type="password"
          value={password}
          onChange={(e) => setPassword(e.target.value)}
          placeholder="Password"
          required
        />
        <button type="submit">Login</button>
      </form>
    </div>
  );
}

export default Login;
