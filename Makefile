CC=gcc
CFLAGS=-fsanitize=address -Wvla -Wall -Werror -s -std=gnu11 -lasan -Os

# fill in all your make rules

vm_x2017: 
	$(CC) $(CFLAGS) vm_x2017.c -o $@

objdump_x2017: 
	$(CC) $(CFLAGS) objdump_x2017.c -o $@

tests:
	echo "tests"

run_tests:
	for testcase in `ls tests/*`;\
	do \
	if [ $${testcase: -3} == ".in" ];\
	then\
			filename=$$(basename "$$testcase" .in);\
			expectedfile=tests/$$filename.out;\
			result=$$(./vm_x2017 $$testcase 2>&1| diff - $$expectedfile);\
			echo $$result;\
			if [ "$$result" == "" ];\
			then\
			echo "$$filename passed";\
			fi;\
	fi;\
	done;\

clean:
	echo "clean"

