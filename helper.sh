
for testcase in `ls tests/*`;\
do \
if [ ${testcase: -4} == ".asm" ];\
then\
        filename=$(basename "$testcase" .asm);\
        expectedfile=tests/${filename}_mil.out;\
        cat $testcase > $expectedfile
fi;\
done;\