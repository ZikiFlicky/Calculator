CC=gcc
CWARNINGS=-Wall
CFLAGS=-std=c99 $(CWARNINGS)
LINK=-lm

OUT=calculator
RM=rm -f

all: main.c calculator.c
	$(CC) $(CFLAGS) -o $(OUT) $^ $(LINK)

clear: $(OUT)
	$(RM) $^

