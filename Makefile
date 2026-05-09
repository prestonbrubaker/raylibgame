# =============================================
# Makefile for raylib Multiplayer Environment
# =============================================

CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -std=c99

# --- Detect operating system ---
ifeq ($(OS),Windows_NT)
    TARGET_SERVER := server.exe
    TARGET_CLIENT := client.exe
    LDFLAGS_SERVER := -lws2_32
    LDFLAGS_CLIENT := -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32
else
    TARGET_SERVER := server
    TARGET_CLIENT := client
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LDFLAGS_CLIENT := -lraylib -lm -ldl -lpthread -lGL -lX11 -lXrandr -lXi
    endif
    ifeq ($(UNAME_S),Darwin)
        LDFLAGS_CLIENT := -lraylib -lm -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    endif
    LDFLAGS_SERVER :=
endif

# --- Build targets ---
all: server client

server: server.c
	$(CC) $(CFLAGS) server.c -o $(TARGET_SERVER) $(LDFLAGS_SERVER)

client: client.c
	$(CC) $(CFLAGS) client.c -o $(TARGET_CLIENT) $(LDFLAGS_CLIENT)

# --- Clean ---
clean:
	rm -f server client server.exe client.exe *.o

# --- Help ---
help:
	@echo "Usage:"
	@echo "  make          - Build both server and client"
	@echo "  make server   - Build only the server"
	@echo "  make client   - Build only the client (requires raylib)"
	@echo "  make clean    - Remove compiled files"
	@echo ""
	@echo "On Windows use MinGW-w64 or MSYS2."

.PHONY: all server client clean help
