FROM ubuntu:latest

RUN apt-get update
RUN apt-get install -y build-essential cmake

WORKDIR /app
COPY . .

RUN g++ -O2 -std=c++17 src/main.cpp -o snake

CMD ["./snake"]
