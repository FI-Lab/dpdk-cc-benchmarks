CFLAGS=-O3
LDFLAGS=
CC=gcc

.PHONY:sub

sub: sm_builder.o acsmx.o acsmx2.o util.o bnfa_search.o

sm_builder.o:
	$(CC) $(CFLAGS) -c sm_builder.c -o $@ $(LDFLAGS)

acsmx.o:
	$(CC) $(CFLAGS) -c acsmx.c -o $@ $(LDFLAGS)

acsmx2.o:
	$(CC) $(CFLAGS) -c acsmx2.c -o $@ $(LDFLAGS)

util.o:
	$(CC) $(CFLAGS) -c util.c -o $@ $(LDFLAGS)

bnfa_search.o:
	$(CC) $(CFLAGS) -c bnfa_search.c -o $@ $(LDFLAGS)


