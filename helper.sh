
for testcase in `ls tests/*`;\
do \
if [ ${testcase: -4} == ".asm" ];\
then\
        #filename=$(basename "$testcase" .asm);\
        #expectedfile=tests/${filename}_mil.out;\
        #cat $testcase > $expectedfile
        echo ${testcase:0:100}
fi;\
done;\

#i=1
#while [ $i != 20 ]
#do 
    #mv tests/${i} tests/test_invalid_random${i}.in
    #echo $i
    #(( i++ ))
#done