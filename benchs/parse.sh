#!/bin/bash

function get_results_first(){
	for line in `cat $1`
	do
		if [[ "$line" != "name index value" ]]
		then
			rm -f temp_`echo $line | awk ' { print $2}'`
			echo $line | awk ' { print "@ ", $1, " & "; printf "%.3f", $3}' >> temp_`echo $line | awk ' { print $2}'`
		fi
	done
}

function get_results(){
	for line in `cat $1`
	do
		if [[ "$line" != "name index value" ]]
		then
			echo $line | awk ' { print " & "; printf "%.3f", $3;}' >> temp_`echo $line | awk ' { print $2}'`
		fi
	done
}

function parse_results(){
	rm -f $2
	touch $2
	output=`grep "format: raw -> " $1`

	for line in $output
	do
		result_file=${line:23}

		raw_output=`cat $result_file`

		bench_name=""

		for raw_line in $raw_output
		do
			if [[ "$raw_line" == *benchmark:* ]]
			then
				index=`expr match "$raw_line" ".*benchmark: "`;
				bench_name=${raw_line:$index};
			fi
			
			if [[ "$4" == "time" ]]
			then
				if [[ "$raw_line" == *reported_time:* ]]
				then
					index=`expr match "$raw_line" ".*reported_time: "`;
					score=${raw_line:$index};
				fi
			fi
			
			if [[ "$4" == "score" ]]
			then
				if [[ "$raw_line" == *ratio:* ]]
				then
					index=`expr match "$raw_line" ".*ratio: "`;
					score=${raw_line:$index};
				fi
			fi
			
			if [[ "$raw_line" == *selected:\ 1 ]]
			then
				echo "${bench_name:4} $score" | tee -a $2
			fi
		done
	done

	cat $2 | sort | awk -f to_latex.awk > $3
}

function calc_variance(){
	output=`grep "format: raw -> " $1`

	for line in $output
	do
		result_file=${line:23}
		raw_output=`cat $result_file`
		bench_name=""

		sum=0.0
		numbers=0

		for raw_line in $raw_output
		do
			if [[ "$raw_line" == *benchmark:* ]]
			then
				index=`expr match "$raw_line" ".*benchmark: "`;
				bench_name=${raw_line:$index};
			fi

			if [[ "$raw_line" == *reported_time:* ]]
			then
				index=`expr match "$raw_line" ".*reported_time: "`;
				score=${raw_line:$index};

				sum=`echo "$sum+$score" | bc -l`;
				numbers=`echo "$numbers+1" | bc -l`;
			fi
		done

		mean=`echo "$sum/$numbers" | bc -l`
		variance=0.0
		
		for raw_line in $raw_output
		do
			if [[ "$raw_line" == *reported_time:* ]]
			then
				index=`expr match "$raw_line" ".*reported_time: "`;
				score=${raw_line:$index};

				difference=`echo "$score-$mean" | bc -l`;
				variance=`echo "$variance+$difference*$difference" | bc -l`;
			fi
		done
		
		variance=`echo "$variance/$numbers" | bc -l`
		std=`echo "sqrt($variance)" | bc -l`
		
		printf "%s mean:%.5f variance:%.5f std:%.5f \n" $bench_name $mean $variance $std
	done
}

source shrc

IFS_BAK=$IFS
IFS=$'\n'

if [[ "$1" == "variance" ]]
then
	echo "Base"
	calc_variance log_base
	echo "Instrumentation"
	calc_variance log_instrumentation
	echo "UCC Sampling"
	calc_variance log_ucc
	echo "LBR Sampling"
	calc_variance log_lbr
else
	echo "Base"
	parse_results log_base results_base bench_base.dat $1

	echo "Instrumentation"
	parse_results log_instrumentation results_instrumentation bench_instrumentation.dat $1

	echo "UCC Sampling"
	parse_results log_ucc results_ucc bench_ucc.dat $1

	echo "LBR Sampling"
	parse_results log_lbr results_lbr bench_lbr.dat $1

	tar czf results.tar.gz bench_*.dat

	#Generate the array

	get_results_first bench_base.dat
	get_results bench_instrumentation.dat
	get_results bench_ucc.dat
	get_results bench_lbr.dat

	for file in temp_*
	do
		echo " & & & & & \\\\ \hline" >> $file
		cat $file | tr "\\n" " "
		echo ""
	done
	
	rm -f temp_*
	rm -f new_temp_*
fi

IFS=$IFS_BAK
