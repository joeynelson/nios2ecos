set -e
echo "Checking a few things in the Cygwin configuration"

export perlbin=`which perl`
if test "$perlbin" != "/usr/bin/perl"; then
	echo "Found $perlbin. Install perl in Cygwin, don't use the one that comes with Quartus"
	exit 1
fi




echo "Nothing obviously wrong discovered with the configuration"
