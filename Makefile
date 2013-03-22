
CFLAGS := -Wall -Wno-pointer-sign -g -Iinc -DTARGET=0
VPATH=src
#LDFLAGS=

objects = main.o utils_tree.o parser.o

jackfish: $(objects)
	  cc -o jackfish $(CFLAGS) $(objects)
clean:
	@rm -f main
	@rm -f *.o
