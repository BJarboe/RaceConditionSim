COMPILER=g++

# -std=c++11  C/C++ variant to use, e.g. C++ 2011
# -Wall       show all warning files
# -g3         include information for symbolic debugger e.g. gdb 
FLAGS=-std=c++11 -Wall -g3 -c

PROGRAM = fooddelivery

# object files
OBJS = $(PROGRAM).o log.o

$(PROGRAM) : $(OBJS)
	$(COMPILER) -pthread -o $(PROGRAM) $^

# object definitions
$(PROGRAM).o : $(PROGRAM).h $(PROGRAM).cpp
	$(COMPILER) $(FLAGS) $(PROGRAM).cpp
	
log.o : log.h log.c
	$(COMPILER) $(FLAGS) log.c

clean :
	rm -f *.o *~ $(PROGRAM)
