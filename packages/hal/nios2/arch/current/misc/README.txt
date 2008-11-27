README.txt
==========

This directory contains two eCos ecm files: redboot_ROM.ecm and redboot_ROMRAM.ecm. These configuration 
files can be imported into the eCos configtool to create project configurations suitable for
generating Redboot executables for programming into flash. These files have been created using the
standard Redboot template, with additional packages added to support networking and flash. In addition
the configurations have been set to boot from flash.

The redboot_ROM.ecm will create a system that executes from flash. The redboot_ROMRAM.ecm configuration
will create a system that will boot from flash, but which will be copied to RAM for execution.

These configurations are suitable for all of the Altera example designs. If you wish to create Redboot
projects for your own custom hardware, you should not use these configuration files. Instead you should
create a project using the Redboot template, and add packages as required. 