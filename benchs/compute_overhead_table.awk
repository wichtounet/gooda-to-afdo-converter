#! /bin/awk -f

BEGIN {
	print "\\hline \n" 
	print "benchmark & base & instr & ucc  & lbr  & I over & U over & L over \\\\ \\hline \\hline \n"
      }

# Ignore lines with less than 10 chars
length($0) < 10 { next; }

/./ { 
	iover = (100 * ($6 / $4 - 1))
	uover = (100 * ($8 / $4 - 1))
	lover = (100 * ($10 / $4 - 1))

	printf "%s & %.2f & %.2f & %.2f & %.2f & ", $2, $4, $6, $8, $10
	printf "%.2f & ", iover
	printf "%.2f & ", uover
	printf "%.2f \\\\ \\hline \n", lover

	sum_a += iover
	sum_b += uover
	sum_c += lover

	count += 1
    }

END {
	mean_a = (sum_a / count)
	mean_b = (sum_b / count)
	mean_c = (sum_c / count)

	print "\\hline \n";
	print "mean & & & & & " 
	printf "%.2f & %.2f & %.2f", mean_a, mean_b, mean_c
	print " \\\\ \\hline \n";
    }
