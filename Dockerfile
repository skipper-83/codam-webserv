# Use an Ubuntu base image
FROM ubuntu:latest

# Install build dependencies
RUN apt-get update && \
    apt-get install -y build-essential cmake

# Copy your source code into the container
COPY . /src
COPY . /include
COPY . /lib
COPY . /test
COPY . CMakeLists.txt
COPY . /build/conf.conf


# Set the working directory
WORKDIR /build

# Compile your program
RUN cmake .. && make
# CMD ["./webserv", "conf.conf"]