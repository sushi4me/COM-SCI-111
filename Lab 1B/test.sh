#!/bin/bash
# Preparation
if [ "${PATH:0:16}" == "/usr/local/cs/bin" ]
then
    true
else
    PATH=/usr/local/cs/bin:$PATH
fi

if [ -e "simpsh" ] 
then
    rm -rf simpsh
fi

make || exit
chmod 744 simpsh

TEMPDIR="testing_folder"
rm -rf $TEMPDIR
mkdir $TEMPDIR
mv simpsh ./$TEMPDIR/
cd $TEMPDIR

# Testing files
cat > READIN.txt <<'EOF'
Hello world! CS 111!
EOF

touch testout.txt
touch testerr.txt
 
printf "TEST 1.1: rdonly, wronly, command --- \t\t"
./simpsh \
--rdonly READIN.txt \
--wronly testout.txt \
--wronly testerr.txt \
--command 0 1 2 cat > /dev/null 2>&1	
diff testout.txt READIN.txt && printf "PASS"
#

printf "\nTEST 1.2: No existing file for wronly --- \t"
./simpsh \
--wronly no_test.txt > /dev/null 2>&1 && \
[ $$? -ne 0 ] || printf "PASS"
#	
	
printf "\nTEST 1.3: Error in command --- \t\t\t"
> testout.txt
> testerr.txt
./simpsh \
--rdonly READIN.txt \
--wronly testout.txt \
--wronly testerr.txt \
--no_such_thing \
--command 0 1 2 cat > /dev/null 2>&1
diff testout.txt READIN.txt && printf "PASS"
#	
	
printf "\n\nTEST 2.1: Test file flagging --- \t\t"
echo "Hello world!" > testout.txt
echo "I should not be here" > testerr.txt
touch empty.txt
./simpsh \
--rdonly READIN.txt \
--append --wronly testout.txt \
--trunc --wronly testerr.txt \
--command 0 1 2 cat > /dev/null 2>&1
cmp -s READIN.txt testout.txt || diff testerr.txt empty.txt && printf "PASS"
#	

printf "\nTEST 2.2: RDWR, waiting & piping --- \t\t"
> testout.txt
> testerr.txt
./simpsh \
--rdonly READIN.txt \
--rdwr testout.txt \
--wronly testerr.txt \
--pipe \
--command 0 4 2 cat \
--command 3 1 2 cat \
--close 3 \
--close 4 \
--wait > /dev/null
cmp -s READIN.txt testout.txt && printf "PASS"
#

printf "\nTEST 2.3: Using closed files --- \t\t"
> testout.txt
- ./simpsh \
--rdonly READIN.txt \
--wronly testout.txt \
--wronly testerr.txt \
--close 2 \
--command 0 1 2 cat > /dev/null 2>testerr.txt
cmp -s testerr.txt empty.txt || printf "PASS"
#

printf "\nTEST 2.4: Ignore --- \t\t\t\t"
> testout.txt
> testerr.txt
./simpsh \
--rdonly READIN.txt \
--wronly testout.txt \
--wronly testerr.txt \
--ignore 11 \
--abort \
--command 0 1 2 cat \
--wait > /dev/null 2>testerr.txt &
kill -11 $!
cmp -s testout.txt empty.txt && printf "PASS"
#
	
printf "\nTEST 2.5: Catch & abort --- \t\t\t"
> testerr.txt
- ./simpsh \
--rdonly READIN.txt \
--wronly testout.txt \
--wronly testerr.txt \
--catch 11 \
--abort \
--command 0 1 2 cat\
--wait > /dev/null 2>testerr.txt
cmp -s testerr.txt empty.txt || printf "PASS"
#

printf "\nTEST 2.6: General --- \n"
./simpsh \
--verbose \
--rdonly READIN.txt \
--pipe \
--pipe \
--trunc --wronly testout.txt \
--trunc --wronly testerr.txt \
--command 0 2 6 cat \
--command 1 4 6 cat \
--close 1 \
--close 2 \
--command 3 5 6 cat \
--close 3 \
--close 4 \
--wait
diff READIN.txt testout.txt && printf "PASS"
#

printf "\n"

