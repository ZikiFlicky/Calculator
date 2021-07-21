CC=gcc
CWARNINGS=
CFLAGS=-std=c99 $(CWARNINGS)
LINK=-lm

OUT_FILE=calculator
OUT=-o $(OUT_FILE)
RM=rm -f

all: src/main.c src/calculator.c
	$(CC) $(CFLAGS) $(OUT) $^ $(LINK)

clear: $(OUT)
	$(RM) $^

