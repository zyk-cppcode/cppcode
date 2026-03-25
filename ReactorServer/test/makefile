all:client server
client:client.cc
	g++ -o client client.cc -I.. -I../spdlog -pthread
server:server.cc
	g++ -o server server.cc -I.. -I../spdlog -pthread
clean:
	rm -rf client server