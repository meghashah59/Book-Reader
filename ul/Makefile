#CC := arm-linux-gnueabihf-gcc
CC := gcc
CFLAGS := -Wall -Werror

book_reader: book_reader.c
	$(CC) $(CFLAGS) -o book_reader book_reader.c -static

clean:
	rm -f book_reader image.jpg read.wav text.txt

