#!/bin/bash
# Docker will call this script to start the services

# Function to log messages
log() {
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] $1"
}

# Start PostgreSQL
log "Starting PostgreSQL..."
service postgresql start
if [ $? -ne 0 ]; then
    log "Failed to start PostgreSQL"
    exit 1
fi
log "PostgreSQL started successfully"

# Start Redis
log "Starting Redis..."
redis-server --protected-mode no &
if [ $? -ne 0 ]; then
    log "Failed to start Redis"
    exit 1
fi
log "Redis started successfully"

# Keep the script running
log "All services started. Keeping container alive..."
tail -f /dev/null

