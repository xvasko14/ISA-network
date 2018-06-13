CC =g++
CFLAGS =-std=c++11 -Wall -pedantic -Wextra
PROJNAME =main.cpp
RESULT =isabot
LOGIN = xvasko14
FILES = Makefile main.h main.cpp manual.pdf Readme

$(RESULT): $(PROJNAME)
	$(CC) $(CFLAGS) $(PROJNAME) -o $(RESULT)

clean:
	rm -f *~
	rm -f $(RESULT)

tar: clean
	tar -cf $(LOGIN).tar $(FILES)

rmtar:
	rm -f $(LOGIN).tar
