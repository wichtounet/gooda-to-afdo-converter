#!/bin/bash

function parse_results(){
	rm -f $2
	touch $2
	output=`grep "format: raw -> " $1`

	for line in $output
	do
		result_file=${line:23}

		raw_output=`cat $result_file`

		bench_name=""

		current_score=0.0;

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

					#current_score=`echo "$current_score+$score" | bc -l`
					
					if [[ $score < $current_score ]]
					then
						current_score=$score
					fi

					if [[ $current_score == 0.0 ]]
					then
						current_score=$score
					fi
				fi
			fi
			
			if [[ "$4" == "score" ]]
			then
				if [[ "$raw_line" == *ratio:* ]]
				then
					index=`expr match "$raw_line" ".*ratio: "`;
					score=${raw_line:$index};
				fi
				
				if [[ "$raw_line" == *selected:\ 1 ]]
				then
					echo "$bench_name $score" | tee -a $2
				fi
			fi
		done

		if [[ "$4" == "time" ]]
		then
			#current_score=`echo "$current_score/3" | bc -l`
			echo "$bench_name $current_score" | tee -a $2
		fi
	done

	cat $2 | sort | awk -f to_latex.awk > $3
}

source shrc

IFS_BAK=$IFS
IFS=$'\n'

echo "Base"
parse_results overhead_base results_overhead_base overhead_base.dat $1

echo "Instrumentation"
parse_results overhead_instrumentation results_overhead_instrumentation overhead_instrumentation.dat $1

echo "UCC Sampling"
parse_results overhead_ucc results_overhead_ucc overhead_ucc.dat $1

echo "LBR Sampling"
parse_results overhead_lbr results_overhead_lbr overhead_lbr.dat $1

mv -f overhead.tar.gz overhead_0.tar.gz
tar czf overhead.tar.gz overhead_*.dat

#Generate the array

function get_results_first(){

for line in `cat $1`
do
	if [[ "$line" != "name index value" ]]
	then
		rm temp_`echo $line | awk ' { print $2}'`
		echo $line | awk ' { print "@ ", $1, " & ", $3}' >> temp_`echo $line | awk ' { print $2}'`
	fi
done

}

function get_results(){

for line in `cat $1`
do
	if [[ "$line" != "name index value" ]]
	then
		echo $line | awk ' { print " & ", $3}' >> temp_`echo $line | awk ' { print $2}'`
	fi
done

}

get_results_first overhead_base.dat
get_results overhead_instrumentation.dat
get_results overhead_ucc.dat
get_results overhead_lbr.dat

for file in temp_*
do
	echo " & & & \\\\ \hline" >> $file
	cat $file | tr "\\n" " "
	echo ""
done

IFS=$IFS_BAK
