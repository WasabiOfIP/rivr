SRCDIR=src
TSTDIR=tests
CC=gcc
CFLAGS=-Wall

DEBUG=no
ifeq ($(DEBUG), yes)
CFLAGS += -D DEBUG -g
endif

PROFILER=no
ifeq ($(PROFILER), yes)
CFLAGS += -pg
endif

ODIR=obj

_DEPS = keywords.h opcodes.h parser.h rv_functions.h rv_strings.h rv_types.h vm.h version.h rv_objects.h
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ = keywords.o parser.o rv_functions.o rv_strings.o vm.o rv_objects.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_MAIN = rivr.o
MAIN = $(patsubst %,$(ODIR)/%,$(_MAIN))

_PW_DEPS = keywords.h opcodes.h rv_functions.h rv_strings.h rv_types.h vm.h version.h rv_objects.h
PW_DEPS = $(patsubst %,$(SRCDIR)/%,$(_PW_DEPS))

_PW_OBJ = pw_prog_writer.o pw_vm.o pw_rv_strings.o pw_rv_functions.o rv_objects.o
PW_OBJ = $(patsubst %,$(ODIR)/%,$(_PW_OBJ))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
	
$(ODIR)/pw_%.o: $(SRCDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

rivr: $(OBJ) $(MAIN)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -rf $(ODIR) *~ core $(INCDIR)/*~ 
	
rivr_d: $(OBJ) $(MAIN)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	
prog_writer: $(PW_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

tests: $(OBJ)
	$(CC) -o test_strings $^ $(TSTDIR)/rv_strings_test.c $(CFLAGS) $(LIBS)
	$(CC) -o test_objects $^ $(TSTDIR)/rv_objects_test.c $(CFLAGS) $(LIBS)