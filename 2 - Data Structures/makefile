CC = gcc
CFLAGS = -g -Wall -std=c99

spellChecker : spellChecker.o hashMap.o
	$(CC) $(CFLAGS) -o $@ $^

hashMap.o : hashMap.h hashMap.c

spellChecker.o : spellChecker.c hashMap.h

memCheckTests :
	valgrind --tool=memcheck --leak-check=yes tests

memCheckSpellChecker :
	valgrind --tool=memcheck --leak-check=yes spellChecker

clean :
	-rm *.o
	-rm spellChecker
