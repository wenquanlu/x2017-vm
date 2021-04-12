#!/bin/bash

for testcase in `ls tests/*`;\
do \
if [ ${testcase: -3} == ".in" ];\
then\
        filename=$(basename "$testcase" .in);\
        expectedfile=tests/$filename.out;\
        result=$(./vm_x2017 $testcase 2>&1| diff - $expectedfile);\
        echo $result;\
        if [ "$result" == "" ];\
        then\
        echo "vm $filename passed";\
        fi;\
        mil_expected=tests/${filename}_mil.out
        result_mil=$(./objdump_x2017 $testcase | diff - $mil_expected)
        echo $result_mil
        if [ "$result_mil" == "" ];\
        then\
        echo "milstone $filename passed";\
        fi;\
fi;\
done;\