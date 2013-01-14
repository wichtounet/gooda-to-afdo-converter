#! /bin/awk -f

BEGIN {
	print "\\hline \n" 
	print "benchmark & base & instr & ucc  & lbr  & I gain & U gain & L gain & U / I & L / I \\\\ \\hline \\hline \n"
      }

# Ignore lines with less than 10 chars
length($0) < 10 { next; }

/./ { 
	igain = (100 * ($6 / $4 - 1))
	ugain = (100 * ($8 / $4 - 1))
	lgain = (100 * ($10 / $4 - 1))
	ui = (100 * (($8 - $4) / ($6 - $4)))
	li = (100 * (($10 - $4) / ($6 - $4)))

	printf "%s & %.2f & %.2f & %.2f & %.2f & ", $2, $4, $6, $8, $10
	printf "%.2f & ", igain
	printf "%.2f & ", ugain
	printf "%.2f & ", lgain
	printf "%.2f & ", ui
	printf "%.2f \\\\ \\hline \n", li

	sum_a += igain
	sum_b += ugain
	sum_c += lgain
	sum_d += ui
	sum_e += li

	count += 1
    }

END {
	mean_a = (sum_a / count)
	mean_b = (sum_b / count)
	mean_c = (sum_c / count)
	mean_d = 100 * (mean_b / mean_a)
	mean_e = 100 * (mean_c / mean_a)

	print "\\hline \n";
	print "mean & & & & & " 
	printf "%.2f & %.2f & %.2f & %.2f & %.2f", mean_a, mean_b, mean_c, mean_d, mean_e 
	print " \\\\ \\hline \n";
    }
