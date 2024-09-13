import Redis from 'ioredis';

const redis = new Redis({
  host: 'localhost', // Change this if Redis is on a different host
  port: 6379,
});

export default redis;