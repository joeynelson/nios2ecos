#
# altera_avalon_sd_mmc_spi_sw.tcl
#

# Create a new driver
create_driver altera_avalon_sd_mmc_spi_driver

# Associate it with some hardware known as "altera_avalon_sd_mmc_spi"
set_sw_property hw_class_name altera_avalon_sd_mmc_spi

# The version of this driver is "1.0"
set_sw_property version 1.0

# This driver is proclaimed to be compatible with altera_avalon_sd_mmc_spis
# as old as version "1.0"
set_sw_property min_compatible_hw_version 1.0

# Initialize the driver in alt_sys_init()
set_sw_property auto_initialize true

# Location in generated BSP that above sources will be copied into
set_sw_property bsp_subdirectory drivers

# Source files
add_sw_property lib_source HAL/lib/libaltera_avalon_sd_mmc_spi.a
add_sw_property include_source HAL/inc/altera_avalon_sd_mmc_spi.h
add_sw_property include_source inc/altera_avalon_sd_mmc_spi_regs.h

# This driver supports HAL & UCOSII BSP (OS) types
add_sw_property supported_bsp_type HAL
add_sw_property supported_bsp_type UCOSII

# Add per-driver configuration option for optional CRC generation
add_sw_setting boolean_define_only public_mk_define use_crc ALTERA_AVALON_SD_MMC_SPI_USE_CRC true "Enable hardware CRC generation to SD/SPI controller"

# End of file
