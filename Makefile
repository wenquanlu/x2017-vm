CC=gcc
CFLAGS=-fsanitize=address -Wvla -Wall -Werror -s -std=gnu11 -lasan -Os
.PHONY: tests
# fill in all your make rules

vm_x2017: 
	$(CC) $(CFLAGS) vm_x2017.c share_func.c -o $@

objdump_x2017: 
	$(CC) $(CFLAGS) objdump_x2017.c share_func.c -o $@

tests:
	$(CC) $(CFLAGS) assembler.c -o assembler
	for testcase in `ls tests/*`;\
	do \
	if [ $${testcase: -4} == ".asm" ];\
	then \
			filename=$$(basename "$$testcase" .asm);\
			assembledfile=tests/$$filename.in;\
			touch $$assembledfile;\
			./assembler $$testcase $$assembledfile;\
	fi;\
	done;\

run_tests:
	bash test.sh

clean:
	rm vm_x2017
	rm objdump_x2017
	rm assembler
	for testcase in `ls tests/*`;\
	do \
	if [ $${testcase: -4} == ".asm" ];\
	then \
		filename=$$(basename "$$testcase" .asm);\
		assembledfile=tests/$$filename.in;\
		rm $$assembledfile;\
	fi;\
	done;\

