include ../Makefile.share
SRC = $(wildcard *.c)
_OBJ = $(SRC:.c=.o)
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LFLAGS)

client: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LFLAGS) $(LIBS)


