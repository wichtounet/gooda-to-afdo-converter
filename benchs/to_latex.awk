#! /bin/awk -f

BEGIN { i = 1; print "name index value"; } ;

 { print $1, " ", i, " ", $2 ; i += 1; }
