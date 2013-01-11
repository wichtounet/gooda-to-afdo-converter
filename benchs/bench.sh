#!/bin/bash

TIMEFORMAT='%3R'
ITERATIONS=1

ucc_iter=1
lbr_iter=1

rm -f bench_converter_ucc.dat
rm -f bench_converter_lbr.dat

rm -f bench_gooda_ucc.dat
rm -f bench_gooda_lbr.dat

echo "name index value" > bench_converter_ucc.dat
echo "name index value" > bench_converter_lbr.dat

echo "name index value" > bench_gooda_ucc.dat
echo "name index value" > bench_gooda_lbr.dat

function bench(){
	cp ../perf-files/$1-$2.perf.data perf.data

	gooda_min=0.0
	converter_min=0.0

	#Gooda
	for i in {0..$ITERATIONS-1}
	do
		rm -rf spreadsheets

		{ time /home/users/atlas/gooda/gooda-analyzer/gooda 2>/dev/null ; } 2> tmp
		result=`cat tmp`

		if [[ $result < $gooda_min ]]
		then
			gooda_min=$result
		fi

		if [[ $gooda_min == 0.0 ]]
		then
			gooda_min=$result
		fi
	done

	printf "gooda %s min:%0.3f \n" $1 $gooda_min
	
	#Converter
	for i in {0..$ITERATIONS-1}
		do
		if [[ $2 == "ucc" ]]
		then
			{ time /home/wichtounet/gcc/google/build/bin/converter --afdo --nows spreadsheets ; } 2>tmp
		elif [[ $2 == "lbr" ]]
		then
			{ time /home/wichtounet/gcc/google/build/bin/converter --lbr --afdo --nows spreadsheets ; } 2>tmp
		fi
		
		result=`cat tmp`

		if [[ $result < $converter_min ]]
		then
			converter_min=$result
		fi

		if [[ $converter_min == 0.0 ]]
		then
			converter_min=$result
		fi
	done

	printf "converter %s min:%0.3f \n" $1 $converter_min
	
	if [[ $2 == "ucc" ]]
	then
		printf "%s %d %0.3f \n" $1 $ucc_iter $converter_min >> bench_converter_ucc.dat
		printf "%s %d %0.3f \n" $1 $ucc_iter $gooda_min >> bench_gooda_ucc.dat
		ucc_iter=$(($ucc_iter + 1))
	elif [[ $2 == "lbr" ]]
	then
		printf "%s %d %0.3f \n" $1 $lbr_iter $converter_min >> bench_converter_lbr.dat
		printf "%s %d %0.3f \n" $1 $lbr_iter $gooda_min >> bench_gooda_lbr.dat
		lbr_iter=$(($lbr_iter + 1))
	fi

	rm tmp
	rm -f fbdata.afdo
	rm -rf spreadsheets
	rm -f perf.data
}

bench gcc-google-converter ucc
bench gcc-google-converter lbr

bench gcc-google-eddic ucc
bench gcc-google-eddic lbr

bench eddic-assembly ucc
bench eddic-assembly lbr

bench eddic-list ucc
bench eddic-list lbr 

bench converter-gcc-ucc ucc
bench converter-gcc-lbr lbr

bench converter-gcc-ucc ucc
bench converter-gcc-lbr lbr

tar czf results.tar.gz bench_converter_ucc.dat bench_converter_lbr.dat bench_gooda_ucc.dat bench_gooda_lbr.dat
