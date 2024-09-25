# Use an official Ubuntu as the base image
FROM ubuntu:20.04

# Set timezone
ENV TZ=Asia/Singapore
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Install necessary packages, including build tools, networking libraries, Redis, and PostgreSQL
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
    postgresql \
    postgresql-contrib \
    libpq-dev \
    libhiredis-dev \
    && apt-get clean

RUN DEBIAN_FRONTEND=noninteractive apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y software-properties-common && \
    DEBIAN_FRONTEND=noninteractive add-apt-repository ppa:redislabs/redis && \
    DEBIAN_FRONTEND=noninteractive apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y redis-server libpqxx-dev

# Install redis-plus-plus
RUN git clone https://github.com/sewenew/redis-plus-plus.git && \
    cd redis-plus-plus && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make && \
    make install && \
    ldconfig && \
    cd ../.. && \
    rm -rf redis-plus-plus

# Set the working directory inside the container
WORKDIR /usr/src/app

# Copy the current directory contents into the container
COPY . .

# Install required Python packages (e.g., redis-py and psycopg2 for PostgreSQL)
RUN pip3 install --no-cache-dir redis psycopg2-binary bcrypt

# Expose Redis and PostgreSQL ports
EXPOSE 6379 5432

# Create a PostgreSQL user and database
USER postgres
RUN /etc/init.d/postgresql start && \
    psql --command "CREATE USER docker WITH SUPERUSER PASSWORD 'docker';" && \
    createdb -O docker docker

# Switch back to root user
USER root

# Add a script to start both Redis and PostgreSQL
COPY start_services.sh /start_services.sh
RUN chmod +x /start_services.sh
