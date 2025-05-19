CC = g++
CFLAGS = -Iinclude -Wall -Wextra -g

SRC = src/main.cpp src/tracker.cpp
OUT = leak_detector

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
