# build gethostbyaddr executable when user executes "make"
gethostbyaddr: gethostbyaddr.o
	$(CC) $(LDFLAGS) gethostbyaddr.o -o gethostbyaddr
gethostbyaddr.o: gethostbyaddr.c
	$(CC) -o $@ $(CFLAGS) -c gethostbyaddr.c
# remove object files and executable when user executes "make clean"
clean:
	rm *.o gethostbyaddr
