CC=gcc
CCFLAGS=-Wall

all: copyit copyit_extracredit

copyit: copyit.c
	$(CC) $(CCFLAGS) $< -o $@

copyit_extracredit: copyit_extracredit.c
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm copyit copyit_extracredit
