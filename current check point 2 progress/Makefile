CC ?= gcc
X := main
P := pthread
all:
	gcc -o $(X)_client $(X)_client.c -l $(P)
	gcc -o $(X)_server $(X)_server.c -l $(P)
clean:
	rm $(X)_client $(X)_server
