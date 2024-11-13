CC := gcc
CFLAGS := -O3 -Wall -Werror -Wextra -Wpedantic -std=c89
SRCDIR := src
OBJDIR := obj
BINDIR := bin

hood_dnc: hood_dnc.o serial.o dirs
	${CC} ${OBJDIR}/hood_dnc.o ${OBJDIR}/serial.o -o ${BINDIR}/hood_dnc

hood_dnc.o: dirs
	${CC} -c ${SRCDIR}/hood_dnc.c ${CFLAGS} -o ${OBJDIR}/hood_dnc.o

serial.o: dirs
	${CC} -c ${SRCDIR}/serial/serial.c ${CFLAGS} -o ${OBJDIR}/serial.o

dirs:
	mkdir $(OBJDIR) --parents
	mkdir $(BINDIR) --parents

clean:
	rm -r $(OBJDIR)
	rm -r $(BINDIR)