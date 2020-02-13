include ../Makefile.share
APPNAME = client
SRC = $(wildcard *.c)
HEAS = $(wildcard *.h)
_OBJ = $(SRC:.c=.o)
OBJ = $(patsubst %,$(ODIR)/$(APPNAME)/%,$(_OBJ))

$(ODIR)/$(APPNAME)/%.o: %.c $(DEPS) $(HEAS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LFLAGS)

$(APPNAME) : $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LFLAGS) $(LIBS)


