set -e

MAIN_DIR=`pwd`
echo $MAIN_DIR
cd $1

echo Running nios2configgen...
nios2configgen --ptf=/home/laurentiu/workspace/zy1000/build/revc/cycloneIII_3c25_niosII_standard_sopc.ptf --cpu=cpu

echo Running ecosconfig new nios2_dev_board...
ecosconfig new nios2_neek minimal
echo Running ecosconfig tree....
ecosconfig import $MAIN_DIR/$2

ecosconfig tree
echo Running make...
make -s -j4
