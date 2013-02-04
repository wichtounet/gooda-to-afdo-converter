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
				echo "${bench_name:4} $score" >> $2
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
		min=0.0
		max=0.0

		for raw_line in $raw_output
		do
			if [[ "$raw_line" == *benchmark:* ]]
			then
				index=`expr match "$raw_line" ".*benchmark: "`;
				bench_name=${raw_line:$index};
			fi

			if [[ "$raw_line" == *ratio:* ]]
			then
				index=`expr match "$raw_line" ".*ratio: "`;
				score=${raw_line:$index};

				if [[ $score < $min ]]
				then
					min=$score;
				elif [[ $min == 0.0 ]]
				then
					min=$score;
				fi
				
				if [[ $score > $max ]]
				then
					max=$score;
				fi

				sum=`echo "$sum+$score" | bc -l`;
				numbers=`echo "$numbers+1" | bc -l`;
			fi
		done

		mean=`echo "$sum/$numbers" | bc -l`
		variance=0.0
		
		for raw_line in $raw_output
		do
			if [[ "$raw_line" == *ratio:* ]]
			then
				index=`expr match "$raw_line" ".*ratio: "`;
				score=${raw_line:$index};

				difference=`echo "$score-$mean" | bc -l`;
				variance=`echo "$variance+$difference*$difference" | bc -l`;
			fi
		done
		
		variance=`echo "$variance/$numbers" | bc -l`
		std=`echo "sqrt($variance)" | bc -l`

		printf "%s:%d mean:%.5f min:%0.5f max:%0.5f variance:%.5f std:%.5f \n" ${bench_name:4} $numbers $mean $min $max $variance $std
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
	#echo "Base"
	parse_results log_base results_base bench_base.dat $1

	#echo "Instrumentation"
	parse_results log_instrumentation results_instrumentation bench_instrumentation.dat $1

	#echo "UCC Sampling"
	parse_results log_ucc results_ucc bench_ucc.dat $1

	#echo "LBR Sampling"
	parse_results log_lbr results_lbr bench_lbr.dat $1

	#Generate the array

	get_results_first bench_base.dat
	get_results bench_instrumentation.dat
	get_results bench_ucc.dat
	get_results bench_lbr.dat

	rm -f table

	for file in temp_*
	do
		echo " & & & & & \\\\ \hline" >> $file
		cat $file | tr "\\n" " " >> table
		echo "" >> table
	done
	
	sed -i 's/astarBigLakes/astar.bigLakes/g' table
	sed -i 's/astarRivers/astar.rivers/g' table
	sed -i 's/bzip2Combined/bzip2.combined/g' table
	sed -i 's/bzip2Program/bzip2.program/g' table
	sed -i 's/soplexRef/soplex.ref/g' table
	sed -i 's/soplexPds50/soplex.pds-50/g' table
	sed -i 's/h264Forebase/h264.forebase/g' table
	sed -i 's/h264Foremain/h264.foremain/g' table
	sed -i 's/h264Sss/h264.sss/g' table

	sort table | awk -vORS= -f compute_table.awk

	rm -f results_base
	rm -f results_instrumentation
	rm -f results_ucc
	rm -f results_lbr

	rm -f bench_base.dat
	rm -f bench_instrumentation.dat
	rm -f bench_ucc.dat
	rm -f bench_lbr.dat
	
	rm -f table
	rm -f temp_*
	rm -f new_temp_*
fi

IFS=$IFS_BAK
