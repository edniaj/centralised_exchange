Gotta run docker container first bro

# Docker commands

docker rm cex_container

docker build -t cex_image .

docker run -it --name cex_container -p 6379:6379 -p 5432:5432 -p 3000:3000 -v /Users/jd/Desktop/centralised_exchange:/usr/src/app cex_image bash

# second terminal 
docker exec -it cex_container bash      