#!/bin/bash
echo "Timer module tests"

#bash tests2.sh

./test_app start A
./test_app delete A
/bin/sleep 1
./test_app stop B
/bin/sleep 1
./test_app start C
./test_app delete B
/bin/sleep 1
./test_app stop_all
