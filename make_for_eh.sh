# !/bin/bash
# run this script with ./compile_and_prog.sh <app name> <tool name> to clean,
# compile and program the board with a binary ready to run intermittently

set -e
if [ $# -lt 2 ]; then
	echo "Need to enter two arguments: app_name tool"
	exit
fi

if [ "$2" != "alpaca" ]; then
	echo "Tool chain $2 is not safe for intermittent execution"
	exit
fi

echo "Start depclean"
make -s apps/$1/bld/$2/depclean
echo "Start dep build"
make -s apps/$1/bld/$2/dep CONSOLE=2 LIBCAPYBARA_CONT_POWER=0
echo "Start build all"
make -s apps/$1/bld/$2/all CONSOLE=2 LIBCAPYBARA_CONT_POWER=0
echo "programming MSP430"
mspdebug -v 2400 -d /dev/ttyACM0 tilib "prog apps/$1/bld/$2/tutorial.out"
