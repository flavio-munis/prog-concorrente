#!/bin/bash

echo "Testing With 1000 Elements"
echo "Normal..."
time ./sum_one 1000

wait
echo ""

echo "Concurrent (3 Threads)..."
time ./sum_one_conc 3 1000

wait
echo ""
echo ""


echo "Testing With 1000000 Elements"
echo "Normal..."
time ./sum_one 1000000

wait
echo ""

echo "Concurrent (3 Threads)..."
time ./sum_one_conc 3 1000000

wait
echo ""
echo ""


echo "Testing With 100000000 Elements"
echo "Normal..."
time ./sum_one 100000000

wait
echo ""

echo "Concurrent (4 Threads)..."
time ./sum_one_conc 4 100000000

wait
echo ""
echo ""


echo "Testing With 500000000 Elements"
echo "Normal..."
time ./sum_one 500000000

wait
echo ""

echo "Concurrent (4 Threads)..."
time ./sum_one_conc 4 500000000

wait
echo ""
echo ""
