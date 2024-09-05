#!/bin/bash

# Files
OUTPUT_FILE_SEQ="out_seq"
OUTPUT_FILE_CONC="out_conc"
TEST_MATRICES="test_matrices"
RESULT_FILE="results.txt"

# Check if result file exists
if [[ -e $RESULT_FILE ]]; 
then
	> results.txt
else
	touch results.txt
fi

# Regexes for parsing output
read_io_regex="Read IO: \K([0-9]+\.[0-9]+)[^s]"
write_io_regex="Write IO: \K([0-9]+\.[0-9]+)[^s]"
mult_regex="Mult.: \K([0-9]+\.[0-9]+)[^s]"
total_regex="Total Time: \K([0-9]+\.[0-9]+)[^s]"


# $1 Total Iterations
# $2 Rows
# $3 Columns
# $4 Type = seq/conc
# $5 if(type != seq) threads
write_median_value_seq() {
	# Init Variables
	read_io=0.0
	write_io=0.0
	mult=0.0
	total_time=0.0

	# Check if test is sequential or concurrent
	if [ "$4" == seq ]; then
		echo "Initializing Sequential Test.."
	else
		echo "Initializing Concurrent Test ($5 Threads).."
	fi
	
	# Multiply Matrice N Times and Sum All Results
	for i in $(seq 1 $1)
	do
		echo "Multiplyng Matrices $i of $1..."

		# Check if test is sequential or concurrent
		if [ "$4" == seq ]; then
		   output=$(./mult_matriz_seq $TEST_MATRICES $OUTPUT_FILE_SEQ)
		else
		   output=$(./mult_matriz_conc $TEST_MATRICES $OUTPUT_FILE_CONC $5)
		fi
		   
		curr_read_io=`echo "$output" | grep -oP "$read_io_regex"`
		curr_write_io=`echo "$output" | grep -oP "$write_io_regex"`
		curr_mult=`echo "$output" | grep -oP "$mult_regex"`
		curr_total_time=`echo "$output" | grep -oP "$total_regex"`

		read_io=$(echo "$read_io + $curr_read_io" | bc)
		write_io=$(echo "$write_io + $curr_write_io" | bc)
		mult=$(echo "$mult + $curr_mult" | bc)
		total_time=$(echo "$total_time + $curr_total_time" | bc)
	done

	# Calculte Median Values and Write it To Result File
	echo "Calculating Median And Writing To File..."
	
	read_io=$(echo "scale=4; $read_io / $1" | bc)
    write_io=$(echo "scale=4; $write_io / $1" | bc)
    mult=$(echo "scale=4; $mult / $1" | bc)
	total_time=$(echo "scale=4; $total_time / $1" | bc)

	# Check if test is sequential or concurrent
	if [ "$4" == seq ]; then
		echo "$2x$3 - $1 Median Values ($4)" >> results.txt 
	else
		echo "$2x$3 - $1 Median Values ($4 - $5 Threads)" >> results.txt 
	fi

	echo "Read: $read_io" >> results.txt
	echo "Write: $write_io" >> results.txt
	echo "Mult.: $mult" >> results.txt
	echo "Total Time: $total_time" >> results.txt
	echo "" >> results.txt

	echo "All Done!"
	echo ""
}

check_solutions() {
	if [[ $(diff out_seq out_conc) ]]; then
		echo "Error! Solutions are NOT Equal!"
	else
		echo "Sequential and Concurrent Solutions Are Equal!"
	fi
}

# $1 - Rows
# $2 - Columns
generate_matrices() {
	echo "Generating $1x$2 Matrices..."
	./gera_matrizes $1 $2 $TEST_MATRICES
	echo "Matrices Generated!"
}

# 500x500 Matrices
generate_matrices 500 500

write_median_value_seq 5 500 500 seq
write_median_value_seq 5 500 500 conc 1
write_median_value_seq 5 500 500 conc 2
write_median_value_seq 5 500 500 conc 4
write_median_value_seq 5 500 500 conc 8

check_solutions
echo ""

# 1000x1000 Matrices
generate_matrices 1000 1000

write_median_value_seq 5 1000 1000 seq
write_median_value_seq 5 1000 1000 conc 1
write_median_value_seq 5 1000 1000 conc 2
write_median_value_seq 5 1000 1000 conc 4
write_median_value_seq 5 1000 1000 conc 8

check_solutions
echo ""

# 1000x1000 Matrices
generate_matrices 2000 2000

write_median_value_seq 5 2000 2000 seq
write_median_value_seq 5 2000 2000 conc 1
write_median_value_seq 5 2000 2000 conc 2
write_median_value_seq 5 2000 2000 conc 4
write_median_value_seq 5 2000 2000 conc 8

check_solutions
echo ""
