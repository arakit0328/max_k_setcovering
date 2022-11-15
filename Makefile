CC = c++
CFLAGS = -Wall -g
FLAGS = -Wall -g
LIBS = -lm
OBJS = SCPv.o rnkc_main.o


rnkc_main: $(OBJS)
	$(CC) $(FLAGS) -o rnkc_main $(OBJS) $(LIBS)
.cpp.o:
	$(CC) $(CFLAGS) -c $<
clean:
	/bin/rm -rf *.o *~ rnkc_main $(OBJS) $(TARGET)
