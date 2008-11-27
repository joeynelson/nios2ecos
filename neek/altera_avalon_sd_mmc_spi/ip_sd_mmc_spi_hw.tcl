#////////////////////////////////////////////////////////////////////////////
#
#       ******                          Module Name: ip_sd_mmc_spi_wrapper 
#   **************                      Date       : 1. Jun. 2005
# ******************                    Author     : Klaus Brunnbauer
#*******************                    Company    : El Camino GmbH
#*********   ***  ***                   Tel.       : +49 (0)8751-8787-0
#******** ***  * *****    ***           WWW        : http://www.elca.de
# ****** ***** * *****   *****          e-mail     : info@elca.de
#  ***** ***** * *****   *****
#     ** ***** * *****   *****          Revision   : 1.1
#        *****   *************
#        *****   ************
#        *************
#         ************
#                *****
#                *****
#                *****
#         *******************
#      *************************        Copyright
#          ******************           (c) 1999-2008 El Camino GmbH
#
#   Description: SD/MMC SPI CLASS
#
#
#
#
#////////////////////////////////////////////////////////////////////////////
#  This code is licensed to, the licensee:
#  NOT LICENSED, EVALUATION VERSION
#  El Camino grants to the licensee a non-transferable, non-exclusive,
#  paid-up license to use this code as follows:
#  - design with, parameterize, synthesize, simulate, compile and implement
#    in ASIC or PLD/FPGA technologies
#  - distribute, sell and otherwise market programming files, executables
#    and/or devices based on this code
#  - you may NOT use this source code except as expressly provided for above
#    or sublicense or transfer this code or rights with respect thereto.

set_source_file "ip_sd_mmc_spi_wrapper.vhd"
set_module "ip_sd_mmc_spi_wrapper"
set_module_description "OpenCore Plus Evaluation License\nThis core is licensed to:\n(a) engage in software evaluation by performing design entry, timing,\n    place and route, compilation, and verification of logic designs for\n    Altera Devices for evaluation purposes only, provided that that YOU\n    must acquire from El Camino a license that specifically permits the\n    programming of Altera Devices for production use prior to doing so, and\n(b) engage in hardware evaluation by programming the IP Function into Altera\n    Devices so long as the Altera Device is continuously connected via a\n    programming cable to a host development computer which is running Altera\n    development tool programmer software, or otherwise the IP Function will\n    operate for a predetermined amount of time, after which the IP Function\n    is automatically disabled and inoperable."

set_module_property className "altera_avalon_sd_mmc_spi"
set_module_property displayName "SD/MMC-Card SPI Interface"
set_module_property instantiateInSystemModule "true"
set_module_property version "1.1"
set_module_property group "El Camino"
set_module_property author "El Camino"
set_module_property iconPath "elca.gif"
set_module_property datasheetURL "http://www.elcamino.de/Downloads/SD_MMC%20SPI%20Core.pdf"
set_module_property editable "false"
set_module_property previewElaborationCallback "elaborate"
set_module_property previewValidationCallback "validate"
set_module_property simulationFiles [ list "ip_sd_mmc_spi.vhd" "ip_sd_mmc_spi_wrapper.vhd" ]
set_module_property synthesisFiles [ list "ip_sd_mmc_spi.vhd" "ip_sd_mmc_spi_wrapper.vhd" ]
set_module_property simulationModelInVerilog "true"
set_module_property simulationModelInVHDL "true"

# Module parameters
add_parameter "ms_clock_divider" "integer" "10000" ""
set_parameter_property "ms_clock_divider" "derived" "true"
set_parameter_property "ms_clock_divider" "visible" "false"

add_parameter "target_clock_divider" "integer" "188" ""
set_parameter_property "target_clock_divider" "derived" "true"
set_parameter_property "target_clock_divider" "visible" "false"

add_parameter "system_clock_frequency" "integer" "80" "The clock frequency in MHz used to drive the Avalon interface (round up to next integer)"
set_parameter_property "system_clock_frequency" "displayName" "Avalon Bus Clock Frequency (MHz, rounded up to integer)"
set_parameter_property "system_clock_frequency" "units" "Megahertz"

add_parameter "spi_clock_frequency" "integer" "25" ""
set_parameter_property "spi_clock_frequency" "displayName" "SPI Clock frequency (MHz)"
set_parameter_property "system_clock_frequency" "units" "Megahertz"
set_parameter_property "spi_clock_frequency" "allowedRanges" "1:50"

# Clock Interface avalon_slave_clock
add_clock_interface "avalon_slave_clock"
set_interface_property "avalon_slave_clock" "externallyDriven" "false"
set_interface_property "avalon_slave_clock" "clockRateKnown" "false"
set_interface_property "avalon_slave_clock" "clockRate" "0"
# Ports in interface avalon_slave_clock
add_port_to_interface "avalon_slave_clock" "clk" "clk"
set_port_direction_and_width "clk" "input" 1
add_port_to_interface "avalon_slave_clock" "reset_n" "reset_n"
set_port_direction_and_width "reset_n" "input" 1

# Interface avalon_slave
add_interface "avalon_slave" "avalon" "slave" "avalon_slave_clock"
set_interface_property "avalon_slave" "isNonVolatileStorage" "false"
set_interface_property "avalon_slave" "burstOnBurstBoundariesOnly" "false"
set_interface_property "avalon_slave" "transparentBridge" "false"
set_interface_property "avalon_slave" "readLatency" "0"
set_interface_property "avalon_slave" "readWaitStates" "1"
set_interface_property "avalon_slave" "isFlash" "false"
set_interface_property "avalon_slave" "holdTime" "0"
set_interface_property "avalon_slave" "printableDevice" "false"
set_interface_property "avalon_slave" "registerIncomingSignals" "false"
set_interface_property "avalon_slave" "readWaitTime" "1"
set_interface_property "avalon_slave" "setupTime" "0"
set_interface_property "avalon_slave" "addressGroup" "0"
set_interface_property "avalon_slave" "interleaveBursts" "false"
set_interface_property "avalon_slave" "addressAlignment" "NATIVE"
set_interface_property "avalon_slave" "isBigEndian" "false"
set_interface_property "avalon_slave" "writeLatency" "0"
set_interface_property "avalon_slave" "writeWaitTime" "1"
set_interface_property "avalon_slave" "timingUnits" "Cycles"
set_interface_property "avalon_slave" "minimumUninterruptedRunLength" "1"
set_interface_property "avalon_slave" "registerOutgoingSignals" "false"
set_interface_property "avalon_slave" "addressSpan" "8"
set_interface_property "avalon_slave" "isMemoryDevice" "false"
set_interface_property "avalon_slave" "linewrapBursts" "false"
set_interface_property "avalon_slave" "alwaysBurstMaxBurst" "false"
set_interface_property "avalon_slave" "writeWaitStates" "1"
set_interface_property "avalon_slave" "maximumPendingReadTransactions" "0"
set_interface_property "avalon_slave" "wellBehavedWaitrequest" "false"
# Ports in interface avalon_slave
add_port_to_interface "avalon_slave" "data_from_cpu" "writedata"
set_port_direction_and_width "data_from_cpu" "input" 16
add_port_to_interface "avalon_slave" "mem_addr" "address"
set_port_direction_and_width "mem_addr" "input" 3
add_port_to_interface "avalon_slave" "read_n" "read_n"
set_port_direction_and_width "read_n" "input" 1
add_port_to_interface "avalon_slave" "spi_select" "chipselect"
set_port_direction_and_width "spi_select" "input" 1
add_port_to_interface "avalon_slave" "write_n" "write_n"
set_port_direction_and_width "write_n" "input" 1
add_port_to_interface "avalon_slave" "data_to_cpu" "readdata"
set_port_direction_and_width "data_to_cpu" "output" 16
add_port_to_interface "avalon_slave" "dataavailable" "dataavailable"
set_port_direction_and_width "dataavailable" "output" 1
add_port_to_interface "avalon_slave" "endofpacket" "endofpacket"
set_port_direction_and_width "endofpacket" "output" 1
add_port_to_interface "avalon_slave" "readyfordata" "readyfordata"
set_port_direction_and_width "readyfordata" "output" 1

# IRQ Interface avalon_slave_irq
add_interface "avalon_slave_irq" "interrupt" "sender" "avalon_slave_clock"
set_interface_property "avalon_slave_irq" "irqScheme" "NONE"
set_interface_property "avalon_slave_irq" "associatedAddressablePoint" "avalon_slave"
# Ports in interface avalon_slave_irq
add_port_to_interface "avalon_slave_irq" "irq" "irq"
set_port_direction_and_width "irq" "output" 1

# Wire Interface avalon_slave_export
add_interface "avalon_slave_export" "conduit" "output" "avalon_slave_clock"
# Ports in interface avalon_slave_export
add_port_to_interface "avalon_slave_export" "DO" "export"
set_port_direction_and_width "DO" "input" 1
add_port_to_interface "avalon_slave_export" "WP" "export"
set_port_direction_and_width "WP" "input" 1
add_port_to_interface "avalon_slave_export" "CDn" "export"
set_port_direction_and_width "CDn" "input" 1
add_port_to_interface "avalon_slave_export" "DI" "export"
set_port_direction_and_width "DI" "output" 1
add_port_to_interface "avalon_slave_export" "SCLK" "export"
set_port_direction_and_width "SCLK" "output" 1
add_port_to_interface "avalon_slave_export" "CSn" "export"
set_port_direction_and_width "CSn" "output" 1

proc elaborate {} {
  global ms_clock_divider 
  global target_clock_divider
  calculate_parameters
  set_parameter_value "target_clock_divider" $target_clock_divider
  set_parameter_value "ms_clock_divider" $ms_clock_divider
}

proc validate {} {
  global ms_clock_divider 
  global target_clock_divider
  global system_clock_frequency
  global spi_clock_frequency
  calculate_parameters

  set actual_clk [ expr $system_clock_frequency / $target_clock_divider  ]
  set clk_err [ expr ($actual_clk - $spi_clock_frequency) / $spi_clock_frequency ]
  set clk_percent_error [ expr $clk_err * 100 ]
  set clk_truncated_percent_error [ expr int($clk_percent_error * 10) / 10.0 ]

  send_message "info" "Actual SPI Rate = $actual_clk MHz,   Error: $clk_truncated_percent_error %%"
}

proc calculate_parameters {} {
  global ms_clock_divider 
  global target_clock_divider
  global system_clock_frequency
  global spi_clock_frequency
  set system_clock_frequency [ get_parameter_value "system_clock_frequency" ]
  set spi_clock_frequency [ get_parameter_value "spi_clock_frequency" ]

  # In TCL, forward slash (/) defaults to integer divide, unless either operand
  # is explicitly non-integer. Here, by adding 0.0, we tell the interpreter to
  # treat these variables as floating point.
  set system_clock_frequency [ expr $system_clock_frequency + 0.0 ]
  set spi_clock_frequency [ expr $spi_clock_frequency + 0.0 ]

  set ms_clock_divider [ expr ceil ($system_clock_frequency * 1000) ]

  set target_clock_divider [ expr ceil($system_clock_frequency / $spi_clock_frequency) ]
  if { $target_clock_divider < 1.0 } { set target_clock_divider 1 }
}
