CC = gcc
CFLAGS = -g -Wall -std=c99

spellChecker : spellChecker.o hashMap.o
	$(CC) $(CFLAGS) -o $@ $^

hashMap.o : hashMap.h hashMap.c

spellChecker.o : spellChecker.c hashMap.h

clean :
	-rm *.o
	-rm spellChecker
