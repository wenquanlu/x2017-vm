#!/bin/bash
export ASAN_OPTIONS=verify_asan_link_order=0
pass_vm=0
fail_vm=0
pass_mil=0
fail_mil=0
for testcase in `ls tests/*`
do
if [ ${testcase: -3} == ".in" ]
then
            filename=$(basename "$testcase" .in)
            if [ ${filename:0:19} != "test_invalid_random" ]
            then
                expectedfile=tests/$filename.out
                result=$(./vm_x2017 $testcase 2>&1| diff - $expectedfile)
                #echo $result
                if [ "$result" == "" ]
                then
                    echo "vm $filename passed"
                    (( pass_vm++ ))
                else
                    echo "vm $filename failed"
                    (( fail_vm++ ))
                fi
            else
                ./vm_x2017 $testcase >/dev/null 2>&1
                if [ $? == 1 ]
                then
                    echo "vm $filename passed"
                    (( pass_vm++ ))
                else    
                    echo "vm $filename failed"
                    (( fail_vm++ ))
                fi

            fi
        if [ ${testcase:0:18} != "tests/test_invalid" ]
        then
            mil_expected=tests/${filename}_mil.out
            result_mil=$(./objdump_x2017 $testcase | diff - $mil_expected)
            #echo $result_mil
            if [ "$result_mil" == "" ]
            then
                echo "milstone $filename passed"
                (( pass_mil++ ))
            else
                echo "milstone $filename failed"
                (( fail_mil++ ))
            fi
        fi
fi
done
echo passed $pass_vm vm testcases, fail $fail_vm.
echo passed $pass_mil milstone testcases, fail $fail_mil.