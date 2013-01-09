#! /bin/awk -f

BEGIN { FS = ","; i = 1; print "name index value"; } ;

 { print $1, " ", i, " ", $8 ; i += 1; }
