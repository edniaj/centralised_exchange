# Use an official Ubuntu as the base image
FROM ubuntu:20.04

# Set timezone
ENV TZ=Asia/Singapore
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Install necessary packages, including build tools, networking libraries, and Redis
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y \
    build-essential \
    g++ \
    clang \
    cmake \
    vim \
    wget \
    git \
    python3 \
    python3-pip \
    libc-dev \
    libc6-dev \
    libstdc++-10-dev \
    libssl-dev \
    && apt-get clean

RUN apt-get install -y software-properties-common && \
    add-apt-repository ppa:redislabs/redis && \
    apt-get update && \
    apt-get install -y redis-server

# Set the working directory inside the container
WORKDIR /usr/src/app

# Copy the current directory contents into the container
COPY . .

# Install required Python packages (e.g., redis-py)
RUN pip3 install --no-cache-dir redis

# Expose Redis port if you intend to run the Redis server
EXPOSE 6379

# Specify the command to run when the container starts
CMD ["bash"]