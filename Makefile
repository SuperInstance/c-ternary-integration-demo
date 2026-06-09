CC       = gcc
CFLAGS   = -std=c99 -Wall -Wextra -pedantic -O2 -march=native
LDFLAGS  = -lm

# Path to c-ternary.h (from construct-coordination sibling directory)
C_TERNARY_INC = ../construct-coordination

.PHONY: all clean run

all: fleet_agent

fleet_agent: fleet_agent.c
	$(CC) $(CFLAGS) -I$(C_TERNARY_INC) -o $@ $< $(LDFLAGS)

run: fleet_agent
	./fleet_agent

clean:
	rm -f fleet_agent *.o
