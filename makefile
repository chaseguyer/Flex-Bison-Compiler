BIN  = c-
CC	 = g++
SRCS = $(BIN).y $(BIN).l struct.h globals.h tree.h tree.c symTab.cpp symTab.h semantic.c semantic.h
OBJS = lex.yy.o $(BIN).tab.o
LIBS = -lm

$(BIN): $(OBJS)
	$(CC) $(CCFLAGS) $(OBJS) $(LIBS) -o $(BIN)

$(BIN).tab.h $(BIN).tab.c: $(BIN).y
	bison -v -t -d $(BIN).y   # -d supplies defines file

lex.yy.c: $(BIN).l $(BIN).tab.h
	flex $(BIN).l  # -d debug

all:
	touch $(SRCS)
	make

clean:
	rm -f $(OBJS) $(BIN) lex.yy.c $(BIN).tab.h $(BIN).tab.c $(BIN).tar *~

pdf:	cminus.y cminus.l makefile
	mktex cminus.y
	mv cminus.pdf cminus-y.pdf
	mktex cminus.l
	mv cminus.pdf cminus-l.pdf
	mktex makefile
	save cminus-l.pdf  cminus-y.pdf makefile.pdf
	rm *.tex

tar:
	tar -cvf $(BIN).tar $(SRCS) makefile  
