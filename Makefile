CC = gcc
CFLAGS = -Wall -Wextra -std=c99

FINAL_BINARY = process_manager

all: $(FINAL_BINARY)

$(FINAL_BINARY): process_manager.o
	$(CC) $(CFLAGS) -o $(FINAL_BINARY) process_manager.o

process_manager.o: process_manager.c
	$(CC) $(CFLAGS) -c process_manager.c

clean:
	rm -f $(FINAL_BINARY) process_manager.o process_manager.bin

run: $(FINAL_BINARY)
	@echo "-> running $(FINAL_BINARY)"
	@./$(FINAL_BINARY)

.PHONY: all clean run
