# !/bin/bash
# run this script with ./sec2.sh <test number>
# to compile and program the board with a binary ready to run intermittently
# The saleae needs to be configured with P1.0 on channel 0, P1.1 on channel 1,
# P1.2 on channel 2, and Vbank on channel 3. channels 0-2 need to be set
# digital (10MS/s is fine), channel 3 needs to be set to 625kS/s. Finally, the
# measurement should run for 20seconds and the output file needs to be written
# in the format: sec2_<test number>_<trial number>.csv with 1 bit per digital
# transition and the analog output written using calibrated voltages.

set -e
if [ $# -lt 1 ]; then
	echo "Don't forget to enter defs"
fi
  
C_FILE=periph_test_blank

def_str=""
for def in "$@"
  do
    def_str="$def_str-D$def "
done
C_FLAGS=$def_str
echo $C_FLAGS

echo "Start depclean"
make -s apps/periph_test_dir/bld/alpaca/depclean
echo "Start dep build"
make -s apps/periph_test_dir/bld/alpaca/dep VERBOSE=1 LIBCAPYBARA_CONT_POWER=0 \
  TEST=$C_FILE
echo "Start build all"
make -s apps/periph_test_dir/bld/alpaca/all VERBOSE=1 LIBCAPYBARA_CONT_POWER=0 \
  TEST=$C_FILE CFLAGS="$C_FLAGS"
echo "programming MSP430"
mspdebug -v 2400 -d /dev/ttyACM0 tilib "prog apps/periph_test_dir/bld/alpaca/tutorial.out"
screen -L /dev/ttyUSB0 115200

