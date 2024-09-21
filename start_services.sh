#!/bin/bash
# Docker will call this script to start the services
service redis-server start
service postgresql start
bash