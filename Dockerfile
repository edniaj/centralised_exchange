# Use an official Ubuntu as the base image
FROM ubuntu:20.04
ENV TZ=Asia/Singapore
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y build-essential g++ clang cmake vim wget git

# Install necessary packages
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    clang \
    cmake \
    vim \
    wget \
    git \
    && apt-get clean

# Set the working directory inside the container
WORKDIR /usr/src/app

# Copy the current directory contents into the container
COPY . .

# Specify the command to run when the container starts
CMD ["bash"]
