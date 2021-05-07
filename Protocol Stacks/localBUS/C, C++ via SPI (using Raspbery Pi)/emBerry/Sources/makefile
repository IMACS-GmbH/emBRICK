INCLUDE  = -I
OBJ = Appl.o bB_master.o bB_EasyAPI.o SPI_Driver.o
DEPS = 

#Any special libraries you are using in your project (e.g. -lbcm2835 -lrt `pkg-config --libs gtk+-3.0` ), or leave blank
LIBS = -lbcm2835 -lrt -pthread

CFLAGS =-I. 

CC = gcc

EXTENSION = .c

%.o: %$(EXTENSION) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)


Appl: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

#Cleanup
.PHONY: clean

clean:
	rm -f *.o *~ core *~ Appl
