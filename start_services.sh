#!/bin/bash
# Docker will call this script to start the services

# Function to log messages
log() {
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] $1"
}

# Start PostgreSQL
log "Starting PostgreSQL..."
service postgresql start

# Start Redis
log "Starting Redis..."
redis-server --protected-mode no &

# Run setup script
log "Running setup script..."
/bin/bash /usr/src/app/script/setup.sh

log "All services started and setup completed successfully"