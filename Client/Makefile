CLIENT=client.bin
SOURCES_API=$(wildcard ../Api/*.c)
SOURCES_NP=$(wildcard ../Api/namedPipe/*.c)
SOURCES_ST=$(wildcard ../Api/socket/*.c)
SOURCES_CLIENT=$(wildcard ./*.c)
SOURCES_DB=$(wildcard ../Database/databaseapi.c)

allST: $(SOURCES_ST) $(SOURCES_CLIENT) $(SOURCES_API) $(SOURCES_DATABASE)
	gcc $(SOURCES_ST) $(SOURCES_CLIENT) $(SOURCES_API) $(SOURCES_DATABASE) -o $(CLIENT) -lsqlite3 -Wall

allNP: clientNP

clientNP: $(SOURCES_NP) $(SOURCES_CLIENT) $(SOURCES_API)  $(SOURCES_DB) 
	gcc $(SOURCES_NP) $(SOURCES_CLIENT) $(SOURCES_API)  $(SOURCES_DB) -o $(CLIENT) -lsqlite3 -Wall

clean:
	rm -rf  *.o *.bin

.PHONY: allNP allST clean clientNP
