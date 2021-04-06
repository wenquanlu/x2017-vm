CC=gcc
CFLAGS=-fsanitize=address -Wvla -Wall -Werror -g -std=gnu11 -lasan

# fill in all your make rules

vm_x2017: 
	$(CC) $(CFLAGS) vm_x2017.c -o $@

objdump_x2017: 
	$(CC) $(CFLAGS) objdump_x2017.c -o $@

tests:
	echo "tests"

run_tests:
	echo "run_tests"

clean:
	echo "clean"

