#!/bin/bash

# Run Redis setup
python3 redis/redis.destroy.py

# Run PostgreSQL setup
python3 postgresql/postgresql.destroy.py