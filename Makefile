CC=g++
CFLAGS=-I/usr/include/postgresql
LDFLAGS=-lpigpio -lrt -lpq -pthread
TARGET=dht_thread
SOURCE=dht0703test.cpp

all: clean $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

clean:
	rm -f $(TARGET)