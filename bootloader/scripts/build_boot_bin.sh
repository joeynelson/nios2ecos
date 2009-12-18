#! /bin/bash
export FACTORY_SOF_OFFSET=0X20000
export APPLICATION_SOF_OFFSET=0X120000
export DEFLATOR_FLASH_OFFSET=0X300000
export APPLICATION_FLASH_OFFSET=0X320000

. ./scripts/set_params.sh
echo "Adding factory FPGA image"
file1_size=`stat -c "%s" $FACTORY_SOF.rbf`
cat >$BOOTLOADER_UPGRADE.bin $FACTORY_SOF.rbf
file1_size=$((file1_size+1))
length1=$((BOOTLOADER_FLASH_OFFSET-FACTORY_SOF_OFFSET))
for i in `seq $file1_size $length1`; do
	printf '\xFF' >> $BOOTLOADER_UPGRADE.bin
done

echo "Adding bootloader"
file2_size=`stat -c "%s" $BOOTLOADER.bin`
cat >>$BOOTLOADER_UPGRADE.bin $BOOTLOADER.bin

echo "Done"
