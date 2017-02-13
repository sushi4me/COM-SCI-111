#!/bin/bash

# UCLA CS 111 Lab 1a testing script, written by Zhaoxing Bu (zbu@cs.ucla.edu)

# 10 test cases, each worth 10 points
# no partial credits, a student either gets 10 or 0 for each test case
# this script automatically checks the first 7 test cases
# the reader has to manually check the last 3 test cases
# please do not run multiple testing scripts at the same time
# only run this on lnxsrv09.seas.ucla.edu please
# any comments, suggestions, problem reports are greatly welcomed

if [ -e "simpsh" ]
then
    rm -rf simpsh
fi

make || exit

chmod 744 simpsh

TEMPDIR="lab1areadergradingtempdir"

rm -rf $TEMPDIR

mkdir $TEMPDIR

mv simpsh ./$TEMPDIR/

cd $TEMPDIR


# create testing files

cat > a0.txt <<'EOF'
Hello world! CS 111!
EOF

cat a0.txt > a1.txt
cat a0.txt > a2.txt
cat a0.txt > a3.txt
cat a0.txt > a4.txt
cat a0.txt > a5.txt

echo "please DO NOT run multiple testing scripts at the same time"
echo "starting grading"

NUM_PASSED=0
NUM_FAILED=0

# in Lab 1a, --rdonly, --wronly, --command, and --verbose.

# test case 1 --rdonly can be called with no error
echo ""
echo "--->test case 1:"
./simpsh --rdonly a1.txt > /dev/null 2>&1
diff a1.txt a0.txt
if [ $? == 0 ];
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 1 passed"
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 1 failed"
fi

# test case 2 --rdonly does not have any default flag: test --create
echo ""
echo "--->test case 2:"
./simpsh --rdonly test2_none_exist.txt > /dev/null 2>&1
if [ -e "test2_none_exist.txt" ]
then
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 2 failed"
else
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 2 passed"
fi

# test case 3 --wronly does not have any default flag: test --trunc
echo ""
echo "--->test case 3:"
./simpsh --wronly a2.txt > /dev/null 2>&1
diff a2.txt a0.txt
if [ $? == 0 ]
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 3 passed"
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 3 failed"
fi

# test case 4 --command test rdonly and wronly
echo ""
echo "--->test case 4:"
touch test4out.txt
touch test4err.txt
./simpsh --rdonly a3.txt --wronly test4out.txt --wronly test4err.txt \
    --command 0 1 2 cat > /dev/null 2>&1
sleep 1
diff test4out.txt a0.txt
if [ $? == 0 ]
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 4 passed"
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 4 failed CHECK what's in test4out.txt"
    echo "may need to rerun this test case with more sleep time"
fi

# test case 5 --command test with wc, ls, ps, blah blah
echo ""
echo "--->test case 5:"
touch test5out.txt
touch test5err.txt
./simpsh --rdonly a4.txt --wronly test5out.txt --wronly test5err.txt \
    --command 0 1 2 wc > /dev/null 2>&1
sleep 1
wc < a0.txt > test5outstd.txt
diff test5out.txt test5outstd.txt
if [ $? == 0 ]
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 5 passed"
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 5 failed CHECK what's in test5out.txt"
    echo "may need to rerun this test case with more sleep time"
fi

# test case 6 test exit status of simpsh
echo ""
echo "--->test case 6:"
./simpsh --wronly test6_none_exist.txt > /dev/null 2>&1
if [ $? != 0 ]
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 6 passed"
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 6 failed"
fi

# test case 7 simpsh should continue to next option when encountered an error
echo ""
echo "-->test case 7:"
touch test7out.txt
touch test7err.txt
./simpsh --rdonly a5.txt --wronly test7out.txt --wronly test7err.txt \
    --coxxx --command 0 1 2 cat > /dev/null 2>&1
sleep 1
diff test7out.txt a0.txt
if [ $? == 0 ]
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 7 passed"
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 7 failed CHECK what's in test7out.txt"
    echo "may need to rerun this test case with more sleep time"
fi

# test case 8 --verbose
echo ""
echo "--->test case 8:"
echo "must show verbose info for --command"
echo "must NOT show verbose info for --rdonly and --wronly"
echo "showing --verbose itself or not is both OK"
touch test8in.txt
touch test8out.txt
touch test8err.txt
touch test8outmessage.txt
touch test8verbose_noneempty.txt
touch test8verbose_empty.txt
./simpsh --rdonly test8in.txt --wronly test8out.txt --wronly test8err.txt \
     --verbose --command 0 1 2 sleep 0.01 > test8outmessage.txt 2> /dev/null
cat test8outmessage.txt | grep "sleep" > test8verbose_noneempty.txt
cat test8outmessage.txt | grep "only" > test8verbose_empty.txt
TEST8_RESULT=0
if [ -s test8verbose_noneempty.txt ]
then
    TEST8_RESULT=0
else
    TEST8_RESULT=1
fi
if [ -s test8verbose_empty.txt ]
then
    TEST8_RESULT=1
fi
if [ $TEST8_RESULT == 0 ]
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 8 passed"
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 8 failed"
fi

# test case 9 test there is no wait
echo ""
echo "--->test case 9:"
echo "simpsh should immediately exits without waiting for 61 seconds"
echo "if you find simpsh waits for many seconds, then definitely failed"
#echo "please check the real time listed below"
#echo "if no time reported, manually run the test case without time command"
touch test9in.txt
touch test9out.txt
touch test9err.txt
touch test9time.txt
touch test9time_real.txt
touch test9time_final.txt
(time ./simpsh --rdonly test9in.txt --wronly test9out.txt --wronly test9err.txt \
    --command 0 1 2 sleep 61) 2> test9time.txt
TEST9_TIME_EXIST=1
if [ -s "test9time.txt" ]
then
    TEST9_TIME_EXIST=1
else
    echo "===no time output, pleaes manually check this test case"
    TEST9_TIME_EXIST=0
fi
cat test9time.txt | grep "real" > test9time_real.txt
if [ -s "test9time_real.txt" ]
then
    TEST9_TIME_EXIST=1
else
    echo "===no time output, pleaes manually check this test case"
    TEST9_TIME_EXIST=0
fi
if [ $TEST9_TIME_EXIST == 1 ]
then
    cat test9time_real.txt | grep "1m" > test9time_final.txt
    if [ -s "test9time_final.txt" ]
    then
        NUM_FAILED=`expr $NUM_FAILED + 1`
        echo "===>test case 9 failed"
    else
        NUM_PASSED=`expr $NUM_PASSED + 1`
        echo "===>test case 9 passed"
    fi
fi

# test case 10 test invalid file descriptor
echo ""
echo "READER MUST MANUALLY CHECK THE RESULTS FOR FOLLOWING TEST CASE"
echo ""
echo "--->test case 10:"
touch test10in.txt
touch test10out.txt
touch test10err.txt
./simpsh --rdonly test10in.txt --wronly test10out.txt --wronly test10err.txt \
    --command 0 1 3 cat > /dev/null 2>test10errormessage.txt
echo "should see invalid file descriptor error message below"
cat test10errormessage.txt

# finished testing
echo ""
echo "grading finished"
echo ""
echo "among first 9 auto graded test cases:"
echo "$NUM_PASSED passed, $NUM_FAILED failed"
NUM_COLLECTED=`expr $NUM_PASSED + $NUM_FAILED`
if [ $NUM_COLLECTED != 9 ]
then
    echo "sum is not 9, check what happend"
fi
