--Legal Notice: (C)2007 Altera Corporation. All rights reserved.  Your
--use of Altera Corporation's design tools, logic functions and other
--software and tools, and its AMPP partner logic functions, and any
--output files any of the foregoing (including device programming or
--simulation files), and any associated documentation or information are
--expressly subject to the terms and conditions of the Altera Program
--License Subscription Agreement or other applicable license agreement,
--including, without limitation, that your use is for the sole purpose
--of programming logic devices manufactured by Altera and sold by Altera
--or its authorized distributors.  Please refer to the applicable
--agreement for further details.


-- turn off superfluous VHDL processor warnings 
-- altera message_level Level1 
-- altera message_off 10034 10035 10036 10037 10230 10240 10030 

library altera;
use altera.altera_europa_support_lib.all;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity ip_sd_mmc_spi_wrapper is
        generic (
                 ms_clock_divider : integer := 80000;
                 target_clock_divider : integer := 8;
                 system_clock_frequency : integer := 80;
                 spi_clock_frequency : integer := 10;
                 spi_clock_frequency_high : integer := 25
                 );
        port (
              -- inputs:
                 signal CDn : IN STD_LOGIC;
                 signal DO : IN STD_LOGIC;
                 signal WP : IN STD_LOGIC;
                 signal clk : IN STD_LOGIC;
                 signal data_from_cpu : IN STD_LOGIC_VECTOR (15 DOWNTO 0);
                 signal mem_addr : IN STD_LOGIC_VECTOR (2 DOWNTO 0);
                 signal read_n : IN STD_LOGIC;
                 signal reset_n : IN STD_LOGIC;
                 signal spi_select : IN STD_LOGIC;
                 signal write_n : IN STD_LOGIC;

              -- outputs:
                 signal CSn : OUT STD_LOGIC;
                 signal DI : OUT STD_LOGIC;
                 signal SCLK : OUT STD_LOGIC;
                 signal data_to_cpu : OUT STD_LOGIC_VECTOR (15 DOWNTO 0);
                 signal dataavailable : OUT STD_LOGIC;
                 signal endofpacket : OUT STD_LOGIC;
                 signal irq : OUT STD_LOGIC;
                 signal readyfordata : OUT STD_LOGIC
              );
end entity ip_sd_mmc_spi_wrapper;


architecture europa of ip_sd_mmc_spi_wrapper is
component ip_sd_mmc_spi is 
           generic (
                    ms_clock_divider : integer := 80000;
                    target_clock_divider : integer := 4;
                    system_clock_frequency : integer := 80;
                    spi_clock_frequency : integer := 20
                    );
           port (
                 -- inputs:
                    signal CDn : IN STD_LOGIC;
                    signal DO : IN STD_LOGIC;
                    signal WP : IN STD_LOGIC;
                    signal clk : IN STD_LOGIC;
                    signal data_from_cpu : IN STD_LOGIC_VECTOR (15 DOWNTO 0);
                    signal mem_addr : IN STD_LOGIC_VECTOR (2 DOWNTO 0);
                    signal read_n : IN STD_LOGIC;
                    signal reset_n : IN STD_LOGIC;
                    signal spi_select : IN STD_LOGIC;
                    signal write_n : IN STD_LOGIC;

                 -- outputs:
                    signal CSn : OUT STD_LOGIC;
                    signal DI : OUT STD_LOGIC;
                    signal SCLK : OUT STD_LOGIC;
                    signal data_to_cpu : OUT STD_LOGIC_VECTOR (15 DOWNTO 0);
                    signal dataavailable : OUT STD_LOGIC;
                    signal endofpacket : OUT STD_LOGIC;
                    signal irq : OUT STD_LOGIC;
                    signal readyfordata : OUT STD_LOGIC
                 );
end component ip_sd_mmc_spi;

                signal internal_CSn :  STD_LOGIC;
                signal internal_DI :  STD_LOGIC;
                signal internal_SCLK :  STD_LOGIC;
                signal internal_data_to_cpu :  STD_LOGIC_VECTOR (15 DOWNTO 0);
                signal internal_dataavailable :  STD_LOGIC;
                signal internal_endofpacket :  STD_LOGIC;
                signal internal_irq :  STD_LOGIC;
                signal internal_readyfordata :  STD_LOGIC;

begin

  --the_ip_sd_mmc_spi, which is an e_instance
  the_ip_sd_mmc_spi : ip_sd_mmc_spi
    generic map(
      ms_clock_divider       => ms_clock_divider,
      target_clock_divider   => target_clock_divider,
      system_clock_frequency => system_clock_frequency,
      spi_clock_frequency    => spi_clock_frequency
    )
    port map(
      CSn => internal_CSn,
      DI => internal_DI,
      SCLK => internal_SCLK,
      data_to_cpu => internal_data_to_cpu,
      dataavailable => internal_dataavailable,
      endofpacket => internal_endofpacket,
      irq => internal_irq,
      readyfordata => internal_readyfordata,
      CDn => CDn,
      DO => DO,
      WP => WP,
      clk => clk,
      data_from_cpu => data_from_cpu,
      mem_addr => mem_addr,
      read_n => read_n,
      reset_n => reset_n,
      spi_select => spi_select,
      write_n => write_n
    );


  --vhdl renameroo for output signals
  CSn <= internal_CSn;
  --vhdl renameroo for output signals
  DI <= internal_DI;
  --vhdl renameroo for output signals
  SCLK <= internal_SCLK;
  --vhdl renameroo for output signals
  data_to_cpu <= internal_data_to_cpu;
  --vhdl renameroo for output signals
  dataavailable <= internal_dataavailable;
  --vhdl renameroo for output signals
  endofpacket <= internal_endofpacket;
  --vhdl renameroo for output signals
  irq <= internal_irq;
  --vhdl renameroo for output signals
  readyfordata <= internal_readyfordata;

end europa;

