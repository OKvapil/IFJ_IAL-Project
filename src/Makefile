# IFJ/IAL project - IFJ26 compiler
# Builds a single executable from all sources in src/.

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -MMD -MP
LDFLAGS = -lm
TARGET = ifj26

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJ) $(DEP)

-include $(DEP)