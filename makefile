all: compile

compile:
	@g++ -w server-slow.cpp -o server
	@g++ -w get-one-file-sig.cpp -o client
	@g++ -w client-shell.cpp -o shell