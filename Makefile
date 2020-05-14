# Projekt: Osy Projekt
# Autor:   Jan Kusák|Kus0054
# Datum:   14.05.2020

CC = g++
CFLAGS = -lrt -pthread
CLIENT = clientsSimulation
FORK = server_fork
THREAD = server_thread

all: $(CLIENT) $(FORK) $(THREAD) 

$(CLIENT): $(CLIENT).cpp library.h
			$(CC) $(CLIENT).cpp -o $(CLIENT)

$(FORK): $(FORK).cpp library.h
			$(CC) $(FORK).cpp $(CFLAGS) -o $(FORK)

$(THREAD): $(THREAD).cpp library.h
			$(CC) $(THREAD).cpp $(CFLAGS) -o $(THREAD)

clean-bin: 
			rm -f $(CLIENT)
			rm -f $(FORK)
			rm -f $(THREAD)

clean-bck:
			rm -f *~ *.bak

clean: clean-bin clean-bck