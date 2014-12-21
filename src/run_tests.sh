#!/bin/sh
#
# 1. Make sure the test suite runs all the tests.
# 2. Run the compiled test suite.
#

count_of_tests=`grep ^test_ ./test.c | grep -v RUN_TEST | wc -l`
count_of_runtests=`grep RUN_TEST.test ./test.c | wc -l`

if [ "$count_of_tests" != "$count_of_runtests" ]; then
	echo "error: found $count_of_tests tests but only $count_of_runtests seem to be queued for execution."
	exit 1
fi

if [ ! -x ./run_tests ]; then
	echo "error: couldn't find run_tests, run \"make clean test\" instead."
	exit 1
fi

./run_tests
