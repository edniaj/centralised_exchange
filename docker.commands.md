## docker exec -it
This command is used to execute a command inside a running Docker container. Let's break it down:

docker exec: This allows you to run a command inside an already running container.
-i: This flag stands for interactive, meaning you can interact with the command inside the container.
-t: This flag allocates a pseudo-TTY, which simulates a terminal. It is necessary when you want to interact with the shell (or other terminal-like applications).

docker exec -it <container_name> bash

## docker build -t
The docker build command is used to create a Docker image from a Dockerfile. The -t flag allows you to tag the image with a specific name, which makes it easier to reference later.

-t: This flag lets you assign a name (and optionally a tag) to the image.
The syntax is name:tag, where tag is optional. If not provided, Docker will default to the tag latest.
docker build -t my-ubuntu-redis-app .

Example:
docker build -t my-ubuntu-redis-app .
In this example:

-t my-ubuntu-redis-app: Tags the image with the name my-ubuntu-redis-app.
.: Refers to the current directory, which contains the Dockerfile to build the image.

## docker ps -a
shows running container

## docker stop <container id or container name>

## docker rm <container id or container name>

## docker run -it --name cex_container -p 6379:6379 -v /Users/jd/Desktop/centralised_exchange:/usr/src/app cex_image bash


Starts the container

## docker exec -it cex_container bash

Attaches terminal to an existing container 

## redis-server --protected-mode no
So that we can use redis insight to view container redis cache