#!/bin/bash
echo "Timer module tests 2"

/bin/sleep 1
./test_app delete A
./test_app start B
/bin/sleep 1
./test_app stop B
/bin/sleep 1
./test_app start C
./test_app delete B
/bin/sleep 1
./test_app stop_all
