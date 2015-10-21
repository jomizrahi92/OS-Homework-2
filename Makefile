# Makefile
#
# Makefile for the cissh assignment for cis704
#
.SUFFIXES:
.SUFFIXES: .o .s .c

ASSIGNMENT	 = cissh
PGM		 = cissh

CFILES		 = 
CFILES		+= cissh.c
CFILES		+= cisshSingleCommand.c
CFILES		+= cisshRedirectedInput.c
CFILES		+= cisshRedirectedOutput.c
CFILES		+= cisshPipe.c

OFILES		 = 
OFILES		+= cissh.o
OFILES		+= cisshSingleCommand.o
OFILES		+= cisshRedirectedInput.o
OFILES		+= cisshRedirectedOutput.o
OFILES		+= cisshPipe.o

HFILES		 =

DISTFILES	 =
DISTFILES	+= ${CFILES}
DISTFILES	+= ${HFILES}
DISTFILES	+= Makefile
DISTFILES	+= testScript.txt
DISTFILES	+= cissh.output
DISTFILES	+= solution.output

CC		 = cc
CFLAGS		 = -g

.c.o:
	${CC} -c ${CFLAGS} -o $@ $<

all: test

test: ${PGM}
	cat testScript.txt | ./${PGM} 2>&1

${PGM}.output:
	cat testScript.txt | ./${PGM} > ${PGM}.output 2>&1

diff: clean ${PGM}.output
	@echo
	@echo "=== Below are the differences between the output of "
	@echo "=== your program and the solution output."
	@echo "=== That is, 'Solution.output' and '${PGM}.output'."
	@echo "=== Only lines that differ in the two files are shown."
	@echo "=== The lines beginning with '<' are from the"
	@echo "=== standard output.  Lines beginning with '>' are"
	@echo "=== from your output."
	@echo "=== "
	@echo "=== There should be no differences between the output"
	@echo "=== from your program and the provided 'Solution.output'."
	@echo "=== "
	-@if diff Solution.output ${PGM}.output ; then \
	      comment="=== Congratulations, your output matches exactly."; \
	  else \
	      comment="=== The differences between your output and the solution are shown above."; \
	  fi; \
	  echo "=== "; \
	  echo "=== End of comparison."; \
	  echo "=== "; \
	  echo $$comment;

${PGM}: clean ${OFILES}
	${CC} -o ${PGM} ${OFILES}

clean:
	rm -f *~ ${OFILES} ${PGM}

distribution:
	rm -rf Distribution
	mkdir Distribution
	cp ${DISTFILES} Distribution
	(cd Distribution; zip ${ASSIGNMENT}.zip *)

tags:
	etags ${CFILES} ${HFILES}
