CFLAGS = -Wall -Wextra -pedantic -g -std=gnu99 -I/local/courses/csse2310/include
LINKED_LIB = -L/local/courses/csse2310/lib -lcsse2310a1

all: uqunscramble
uqunscramble: uqunscramble.c
	gcc $(CFLAGS) -o uqunscramble uqunscramble.c $(LINKED_LIB)

clean:
	rm uqunscramble
