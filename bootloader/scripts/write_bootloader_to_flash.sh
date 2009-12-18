. ./scripts/set_params.sh

bin2flash --location=$FACTORY_SOF_OFFSET --input=$BOOTLOADER_UPGRADE.bin --output=$BOOTLOADER_UPGRADE.srec

nios2-flash-programmer --base=$FLASH_BASE $BOOTLOADER_UPGRADE.srec


