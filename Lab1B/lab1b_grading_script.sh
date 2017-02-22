#!/bin/bash

# UCLA CS 111 Lab 1b testing script, written by Zhaoxing Bu (zbu@cs.ucla.edu)

# to reader: please read the entire script carefully

# 10 test cases, each worth 10 points
# no partial credits, a student either gets 10 or 0 for each test case
# this script automatically checks the first 9 test cases
# the reader has to manually check the last 1 test cases
# please do not run multiple testing scripts at the same time
# only run this on lnxsrv09.seas.ucla.edu please
# REMEMBER to execute PATH=/usr/local/cs/bin:$PATH in shell to call
# the correct gcc for compiling students' work
# this PATH change is restored after logging out
# this script automatically changes the PATH value for you
# but I do not guarantee it works, but if should work ;-)
# any comments, suggestions, problem reports are greatly welcomed

if [ "${PATH:0:16}" == "/usr/local/cs/bin" ]
then
    true
else
    PATH=/usr/local/cs/bin:$PATH
fi


echo "please check if there is any error message below"
echo "==="

if [ -e "simpsh" ]
then
    rm -rf simpsh
fi

make || exit

# chmod 744 simpsh

TEMPDIR="lab1breadergradingtempdir"

rm -rf $TEMPDIR

mkdir $TEMPDIR

if [ "$(ls -A $TEMPDIR 2> /dev/null)" == "" ]
then
    true
else
    echo "fatal error! the testing directory is not empty"
    exit 1
fi

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

running_simpsh_check () {
    if ps | grep "simpsh"
    then
        echo "simpsh is running in background"
        echo "testing cannot continue"
        echo "kill it and then run the following test cases"
        exit 1
    fi
}

echo "==="

echo "please DO NOT run multiple testing scripts at the same time"
echo "make sure there is no simpsh running by you"
echo "infinite waiting of simpsh due to unclosed pipe is unacceptable"
echo "starting grading"

running_simpsh_check

NUM_PASSED=0
NUM_FAILED=0

# in Lab 1b, --rdwr, --pipe, --wait, --close, --abort, --catch, --ignore
# --default, --pause, and various file flags

# test case 1 file flags
echo ""
echo "--->test case 1:"
echo "It then waits for all three subprocesses to finish." > test1out.txt
./simpsh --rdonly a1.txt --trunc --wronly test1out.txt --creat --wronly \
    test1err.txt --command 0 1 2 cat > /dev/null 2>&1
sleep 1
diff test1out.txt a0.txt
TEST1_RESULT=$?
if [ -e test1err.txt ]
then
    TEST1_RESULT=`expr $TEST1_RESULT + 0`
else
    TEST1_RESULT=`expr $TEST1_RESULT + 1`
fi
if [ $TEST1_RESULT == 0 ]
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 1 passed"
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 1 failed CHECK what's in test1out.txt"
    echo "may need to rerun this test case with more sleep time"
fi

# test case 2 --pipe works fine
echo ""
echo "--->test case 2:"
echo "if simpsh hangs forever, then this test case is failed"
touch test2out.txt
touch test2err.txt
./simpsh --rdonly a2.txt --wronly test2err.txt --pipe --wronly test2out.txt \
    --command 0 3 1 cat --command 2 4 1 cat > /dev/null 2>&1
sleep 1
diff test2out.txt a0.txt
if [ $? == 0 ]
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 2 passed"
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 2 failed CHECK what's in test2out.txt"
    echo "may need to rerun this test case with more sleep time"
fi

# test case 3 --abort works fine
echo ""
echo "--->test case 3:"
touch test3out.txt
touch test3err.txt
./simpsh --rdonly a3.txt --wronly test3out.txt --wronly test3err.txt \
    --abort --command 0 1 2 cat > /dev/null 2>&1
sleep 1
if [ -s "test3out.txt" ]
then
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 3 failed"
else
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 3 passed"
fi

# test case 4 --pipe, --wait, --close all work fine together, no infinite wait
echo ""
echo "--->test case 4:"
echo "if simpsh hangs forever, then this test case is failed"
touch test4out.txt
touch test4err.txt
./simpsh --rdonly a4.txt --wronly test4out.txt --wronly test4err.txt --pipe \
    --command 0 4 2 cat --command 3 1 2 cat --close 3 --close 4 --wait \
    > /dev/null 2>&1
diff test4out.txt a0.txt
if [ $? == 0 ]
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 4 passed"
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 4 failed"
fi

# test case 5 --wait works fine
echo ""
echo "--->test case 5:"
#echo "simpsh should exits after waiting for 2 seconds"
#echo "please check the real time listed below (> 2 sceonds)"
#echo "if no time reported, manually run the test case without time command"
#echo "should also see a list of --command' exit status, command, and arguments"
touch test5in.txt
touch test5out.txt
touch test5err.txt
touch test5time.txt
touch test5time_real.txt
touch test5outmessage.txt
touch test5outmessage_grep.txt
(time ./simpsh --rdonly test5in.txt --wronly test5out.txt --wronly test5err.txt \
    --command 0 1 2 sleep 2 --wait) 2> test5time.txt > test5outmessage.txt
TEST5_RESULT=1
TEST5_TIME_EXIST=1
if grep -q "real" test5time.txt
then
    cat test5time.txt | grep "real" > test5time_real.txt
else
    echo "===no time output, pleaes manually check this test case"
    TEST5_TIME_EXIST=0
fi
if [ $TEST5_TIME_EXIST == 1 ]
then
    if grep -q "0m0\|0m1" test5time_real.txt
    then
        TEST5_RESULT=0
    fi
    if grep -q "sleep" test5outmessage.txt
    then
        true
    else
        TEST5_RESULT=0
    fi
    if [ $TEST5_RESULT == 1 ]
    then
        NUM_PASSED=`expr $NUM_PASSED + 1`
        echo "===>test case 5 passed"
    else
        NUM_FAILED=`expr $NUM_FAILED + 1`
        echo "===>test case 5 failed"
    fi
fi

# test case 6 --catch signal
echo ""
echo "--->test case 6:"
# running_simpsh_check

touch test6in.txt
touch test6out.txt
touch test6err.txt
touch test6errormessage.txt
touch test6ps.txt
./simpsh --rdonly test6in.txt --wronly test6out.txt --wronly test6err.txt \
    --catch 10 --command 0 1 2 sleep 20 --wait > /dev/null \
    2> test6errormessage.txt &
sleep 1
kill -10 $!
sleep 1
TEST6_RESULT=1
if grep -q "10" test6errormessage.txt
then
    true
else
    TEST6_RESULT=0
fi
if ps | grep -q "$!"
then
    TEST6_RESULT=0
fi
if [ $TEST6_RESULT == 1 ]
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 6 passed"
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 6 failed"
fi

# test case 7 --ignore signal
echo ""
echo "--->test case 7:"
# running_simpsh_check

touch test7in.txt
touch test7out.txt
touch test7err.txt
./simpsh --rdonly test7in.txt --wronly test7out.txt --wronly test7err.txt \
    --ignore 10 --command 0 1 2 sleep 20 --wait > /dev/null 2>&1 &
sleep 1
kill -10 $!
sleep 1
TEST7_RESULT=1
IGNORE_RESULT=0
if ps | grep -q "$!"
then
    true
else
    TEST7_RESULT=0
fi
kill -9 $!
sleep 1
if ps | grep -q "$!"
then
    TEST7_RESULT=0
fi
if [ $TEST7_RESULT == 1 ]
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 7 passed"
    IGNORE_RESULT=1
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 7 failed"
fi

# test case 8 --default signal
# this test assumes the student's --ignore works
# can also depends on --catch instead of --ignore
# but I assume it is hardly to see a student's --catch works but --ignore not
echo ""
echo "--->test case 8:"
# running_simpsh_check

echo "this test case assumes the student's --ignore works"
if [ $IGNORE_RESULT == 1 ]
then
    touch test8in.txt
    touch test8out.txt
    touch test8err.txt
    ./simpsh --ignore 10 --rdonly test8in.txt --wronly test8out.txt \
        --wronly test8err.txt --default 10 --command 0 1 2 sleep 20 \
        --wait > /dev/null 2>&1 &
    sleep 1
    TEST8_RESULT=1
    if ps | grep -q "$!"
    then
        true
    else
        TEST8_RESULT=0
    fi
    kill -10 $!
    sleep 1
    if ps | grep -q "$!"
    then
        TEST8_RESULT=0
    fi
    if [ $TEST8_RESULT == 1 ]
    then
        NUM_PASSED=`expr $NUM_PASSED + 1`
        echo "===>test case 8 passed"
    else
        NUM_FAILED=`expr $NUM_FAILED + 1`
        echo "===>test case 8 failed"
    fi
else
    echo "===>--ignore test failed, this test case automatically faled"
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 8 failed"
fi

# test case 9 --pause
echo ""
echo "--->test case 9:"
# running_simpsh_check

./simpsh --pause &
TEST9_RESULT=1
sleep 1
if ps | grep -q "$!"
then
    true
else
    TEST9_RESULT=0
fi
kill -10 $!
sleep 1
if ps | grep -q "$!"
then
    TEST9_RESULT=0
fi
if [ $TEST9_RESULT == 1 ]
then
    NUM_PASSED=`expr $NUM_PASSED + 1`
    echo "===>test case 9 passed"
else
    NUM_FAILED=`expr $NUM_FAILED + 1`
    echo "===>test case 9 failed"
fi

# test case 10 --close works fine, there is an error message for using closed fd
echo ""
echo "READER MUST MANUALLY CHECK THE RESULTS FOR FOLLOWING TEST CASES"
echo ""
echo "--->test case 10:"
touch test10in.txt
touch test10out.txt
touch test10err.txt
./simpsh --rdonly test10in.txt --wronly test10out.txt --wronly test10err.txt \
    --close 2 --command 0 1 2 cat > /dev/null 2>test10errormessage.txt
echo "should see invalid file descriptor error message below"
echo "==="
cat test10errormessage.txt
echo "==="

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

echo ""
echo "student's README file"
echo "==="
cat ../README
echo "==="
