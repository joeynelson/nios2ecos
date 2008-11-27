Overview:

- This design is based on the Nios II/f core and provides a typical mix of 
  peripherals and memories.

- This is a list of the components available in the system along with a brief 
  description:
  cpu Nios2/f
  pipeline_bridge_before_tristate_bridge
  flash_tristate_bridge
  ext_flash parallel flash
  ddr2_sdram
  cpu_ddr_clock_bridge

- These bridges are here for two reasons:
  1) Clock cross between two components that are clocked at a different 
     frequency.
     * tse_ddr_clock_bridge
     * tse_ssram_clock_bridge for board 3c25

  2) To offset addresses so that the entire memory model is flat. 
     (Every master sees every slave at the same address). 
     * descriptor_offset_bridge

  - The triple speed Ethernet components (for board 3c25, those components are on slow_peripheral_bridge):
    * tse_mac
    * sgdma_tx
    * sgdma_rx
    * descriptor_memory

Components on the slow peripheral bridge
=========================================
- Components listed here are partitioned off by the use of the 
  slow_peripheral_bridge. This allows non-fmax critical components to be 
  clocked at a slower clock rate to give better fitting for components that 
  are fmax-critical.

  - The Bridge
    * slow_peripheral_bridge

  - The rest of the components on the slow_peripheral_bridge
    * sys_clk_timer 
    * performance_counter
    * jtag_uart
    * sysid
    * pll
    * button_pio
    * led_pio

Further Notes:
==============
- This Quartus II project contains assignments that match the port names 
  produced by SOPC Builder. If you add or modify SOPC Builder components, the 
  pin assignments may no longer be valid.  To view the Assignment Editor in the
  Quartus II software, in the Assignments menu, click "Assignment Editor".

- Hardcoded library paths will be inserted into the Quartus settings for this 
  project during system generation due to the presence of the DDR and TSE 
  components.  If you intend to modify the hardware and regenerate a system with
  these components, please remove any old paths from within the project settings
  file (qsf) IF the project directory has been changed.

  An example of the absolute paths are shown below:
     set_global_assignment -name USER_LIBRARIES "C:/niosII_cycloneII_2c35/full_featured;"
     set_global_assignment -name VHDL_FILE "C:/MegaCore/ddr_ddr2_sdram-v3.2.0/lib/auk_ddr_tb_functions.vhd"
     set_global_assignment -name VHDL_FILE "C:/MegaCore/ddr_ddr2_sdram-v3.2.0/lib/auk_ddr_functions.vhd"
     set_global_assignment -name VHDL_FILE "C:/MegaCore/ddr_ddr2_sdram-v3.2.0/lib/auk_ddr_input_buf.vhd"
     set_global_assignment -name VHDL_FILE "C:/MegaCore/ddr_ddr2_sdram-v3.2.0/lib/auk_ddr_timers.vhd"
     set_global_assignment -name VHDL_FILE "C:/MegaCore/ddr_ddr2_sdram-v3.2.0/lib/auk_ddr_avalon_if.vhd"
     set_global_assignment -name VHDL_FILE "C:/MegaCore/ddr_ddr2_sdram-v3.2.0/lib/auk_ddr_bank_details.vhd"
     set_global_assignment -name VHDL_FILE "C:/MegaCore/ddr_ddr2_sdram-v3.2.0/lib/auk_ddr_controller.vhd"
     set_global_assignment -name VHDL_FILE "C:/MegaCore/ddr_ddr2_sdram-v3.2.0/lib/auk_ddr_init.vhd"
     set_global_assignment -name VERILOG_FILE "C:/niosII_cycloneII_2c35/full_featured/ddr_sdram_auk_ddr_dqs_group.v"
     set_global_assignment -name VERILOG_FILE "C:/niosII_cycloneII_2c35/full_featured/ddr_sdram_auk_ddr_clk_gen.v"
     set_global_assignment -name VERILOG_FILE "C:/niosII_cycloneII_2c35/full_featured/ddr_sdram_auk_ddr_datapath.v"
     set_global_assignment -name VERILOG_FILE "C:/niosII_cycloneII_2c35/full_featured/ddr_sdram_auk_ddr_sdram.v"
     set_global_assignment -name VERILOG_FILE "C:/niosII_cycloneII_2c35/full_featured/ddr_sdram.v"

     * These paths have been pre-removed in the qsf of the designs shipped in 
       the installation.

- For board 3c120, only single bank DDR memory is used as program and data memory. Thus, memtest should not run on
  the DDR memory bank as could cause destructive effect to the program code and data.

- The current version of the Nios II EDS hardware design example uses an HDL 
  file as the top level of the design hierarchy.  If you would like to use a 
  schematic-based top level instead (BDF), follow the steps listed below.  
    1) In the Quartus II software, open the top-level HDL file (.v or .vhd) for
       the design.
    2) Create a symbol for the HDL file by clicking 
       File -> Create/Update -> Create Symbol Files for Current File
    3) Create a new BDF file by clicking 
       File -> New -> Block Diagram/Schematic File.
    4) Instantiate the symbol in the BDF by double-clicking in the empty space
       of the BDF file and selecting "Project -> <symbol filename>"
    5) Instantiate pins in the BDF by double-clicking empty space, then 
       typing "input", "output", or "bidir".
    6) Rename the pins and connect them to the appropriate ports on the symbol.
    7) Save the BDF as a unique filename.
    8) Set the BDF as your top level entity by clicking:
       Project -> Set as Top-Level Entity
    9) Recompile the Quartus II project.
    
- To compile this software in Nios II IDE, you need to append this Project directory or the directory
  that contains the altera_avalon_sd_mmc_spi component to the "SOPC_BUILDER_PATH" Environment Variable, 
  so that the build tools will be able to locate the drivers for the SD controller.

  For more information and details, refer to the Nios II Embedded Design Suite 
  Release Note.
