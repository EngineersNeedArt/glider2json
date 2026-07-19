CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -std=c11
TARGET = glider2json
SRCS = main.c HouseConvert.c cJSON.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS) HouseConvert.h cJSON.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
