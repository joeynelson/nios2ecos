//Legal Notice: (C)2008 Altera Corporation. All rights reserved.  Your
//use of Altera Corporation's design tools, logic functions and other
//software and tools, and its AMPP partner logic functions, and any
//output files any of the foregoing (including device programming or
//simulation files), and any associated documentation or information are
//expressly subject to the terms and conditions of the Altera Program
//License Subscription Agreement or other applicable license agreement,
//including, without limitation, that your use is for the sole purpose
//of programming logic devices manufactured by Altera and sold by Altera
//or its authorized distributors.  Please refer to the applicable
//agreement for further details.

// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module cycloneIII_3c25_niosII_standard (
                                         // inputs:
                                          top_button,
                                          top_clkin_50,
                                          top_reset_n,

                                         // outputs:
                                          top_flash_cs_n,
                                          top_flash_oe_n,
                                          top_flash_reset_n,
                                          top_flash_ssram_a,
                                          top_flash_ssram_d,
                                          top_flash_wr_n,
                                          top_led,
                                          top_mem_addr,
                                          top_mem_ba,
                                          top_mem_cas_n,
                                          top_mem_cke,
                                          top_mem_clk,
                                          top_mem_clk_n,
                                          top_mem_cs_n,
                                          top_mem_dm,
                                          top_mem_dq,
                                          top_mem_dqs,
                                          top_mem_ras_n,
                                          top_mem_we_n,
                                          top_ssram_adsc_n,
                                          top_ssram_bw_n,
                                          top_ssram_bwe_n,
                                          top_ssram_ce_n,
                                          top_ssram_clk,
                                          top_ssram_oe_n,
                                          
                                          // OpenCores Ethernet core
                                          top_mcoll_pad_i_to_the_ocm,
                                          top_mcrs_pad_i_to_the_ocm,
                                          top_mdio_pad_io_to_the_ocm,
                                          top_mdc_pad_o_from_the_ocm,
                                          top_mrx_clk_pad_i_to_the_ocm,
                                          top_mrxd_pad_i_to_the_ocm,
                                          top_mrxdv_pad_i_to_the_ocm,
                                          top_mrxerr_pad_i_to_the_ocm,
                                          top_mtx_clk_pad_i_to_the_ocm,
                                          top_mtxd_pad_o_from_the_ocm,
                                          top_mtxen_pad_o_from_the_ocm,
                                          top_mtxerr_pad_o_from_the_ocm,
                                          top_HC_ETH_RESET_N
                                       );

  // New OpenCores Ethernet core signals
  input            top_mcoll_pad_i_to_the_ocm;
  input            top_mcrs_pad_i_to_the_ocm;
  inout            top_mdio_pad_io_to_the_ocm;
  output           top_mdc_pad_o_from_the_ocm;
  input            top_mrx_clk_pad_i_to_the_ocm;
  input      [3:0] top_mrxd_pad_i_to_the_ocm;
  input            top_mrxdv_pad_i_to_the_ocm;
  input            top_mrxerr_pad_i_to_the_ocm;
  input            top_mtx_clk_pad_i_to_the_ocm;
  output     [3:0] top_mtxd_pad_o_from_the_ocm;
  output           top_mtxen_pad_o_from_the_ocm;
  output           top_mtxerr_pad_o_from_the_ocm;
  output           top_HC_ETH_RESET_N;

  output           top_flash_cs_n;
  output           top_flash_oe_n;
  output           top_flash_reset_n;
  output  [ 23: 0] top_flash_ssram_a;
  inout   [ 31: 0] top_flash_ssram_d;
  output           top_flash_wr_n;
  output  [  1: 0] top_led;
  output  [ 12: 0] top_mem_addr;
  output  [  1: 0] top_mem_ba;
  output           top_mem_cas_n;
  output           top_mem_cke;
  inout            top_mem_clk;
  inout            top_mem_clk_n;
  output           top_mem_cs_n;
  output  [  1: 0] top_mem_dm;
  inout   [ 15: 0] top_mem_dq;
  inout   [  1: 0] top_mem_dqs;
  output           top_mem_ras_n;
  output           top_mem_we_n;
  output           top_ssram_adsc_n;
  output  [  3: 0] top_ssram_bw_n;
  output           top_ssram_bwe_n;
  output           top_ssram_ce_n;
  output           top_ssram_clk;
  output           top_ssram_oe_n;
  input   [  3: 0] top_button;
  input            top_clkin_50;
  input            top_reset_n;

  // New OpenCores Ethernet core signals
  wire             top_mcoll_pad_i_to_the_ocm;
  wire             top_mcrs_pad_i_to_the_ocm;
  wire             top_mdio_pad_io_to_the_ocm;
  wire             top_mdc_pad_o_from_the_ocm;
  wire             top_mrx_clk_pad_i_to_the_ocm;
  wire       [3:0] top_mrxd_pad_i_to_the_ocm;
  wire             top_mrxdv_pad_i_to_the_ocm;
  wire             top_mrxerr_pad_i_to_the_ocm;
  wire             top_mtx_clk_pad_i_to_the_ocm;
  wire       [3:0] top_mtxd_pad_o_from_the_ocm;
  wire             top_mtxen_pad_o_from_the_ocm;
  wire             top_mtxerr_pad_o_from_the_ocm;
  wire             top_md_pad_i_to_the_ocm;
  wire             top_md_pad_o_from_the_ocm;
  wire             top_md_padoe_o_from_the_ocm;
  wire             top_HC_ETH_RESET_N;

  wire             top_clk_to_offchip_video;
  wire             top_ddr_sdram_aux_full_rate_clk_out;
  wire             top_ddr_sdram_aux_half_rate_clk_out;
  wire             top_ddr_sdram_phy_clk_out;
  wire             top_flash_cs_n;
  wire             top_flash_oe_n;
  wire             top_flash_reset_n;
  wire    [ 23: 0] top_flash_ssram_a;
  wire    [ 31: 0] top_flash_ssram_d;
  wire             top_flash_wr_n;
  wire    [  3: 0] top_in_port_to_the_button_pio;
  wire    [  1: 0] top_led;
  wire             top_local_init_done_from_the_ddr_sdram;
  wire             top_local_refresh_ack_from_the_ddr_sdram;
  wire             top_local_wdata_req_from_the_ddr_sdram;
  wire    [ 12: 0] top_mem_addr;
  wire    [  1: 0] top_mem_ba;
  wire             top_mem_cas_n;
  wire             top_mem_cke;
  wire             top_mem_clk;
  wire             top_mem_clk_n;
  wire             top_mem_cs_n;
  wire    [  1: 0] top_mem_dm;
  wire    [ 15: 0] top_mem_dq;
  wire    [  1: 0] top_mem_dqs;
  wire             top_mem_ras_n;
  wire             top_mem_we_n;
  wire             top_peripheral_clk;
  wire             top_remote_update_clk;
  wire             top_reset_phy_clk_n_from_the_ddr_sdram;
  wire             top_ssram_adsc_n;
  wire    [  3: 0] top_ssram_bw_n;
  wire             top_ssram_bwe_n;
  wire             top_ssram_ce_n;
  wire             top_ssram_clk;
  wire             top_ssram_oe_n;
  cycloneIII_3c25_niosII_standard_sopc cycloneIII_3c25_niosII_standard_sopc_instance
    (
      .mcoll_pad_i_to_the_ocm (top_mcoll_pad_i_to_the_ocm),
      .mcrs_pad_i_to_the_ocm (top_mcrs_pad_i_to_the_ocm),
      .md_pad_i_to_the_ocm (top_md_pad_i_to_the_ocm),
      .md_pad_o_from_the_ocm (top_md_pad_o_from_the_ocm),
      .md_padoe_o_from_the_ocm (top_md_padoe_o_from_the_ocm),
      .mdc_pad_o_from_the_ocm (top_mdc_pad_o_from_the_ocm),
      .mrx_clk_pad_i_to_the_ocm (top_mrx_clk_pad_i_to_the_ocm),
      .mrxd_pad_i_to_the_ocm (top_mrxd_pad_i_to_the_ocm),
      .mrxdv_pad_i_to_the_ocm (top_mrxdv_pad_i_to_the_ocm),
      .mrxerr_pad_i_to_the_ocm (top_mrxerr_pad_i_to_the_ocm),
      .mtx_clk_pad_i_to_the_ocm (top_mtx_clk_pad_i_to_the_ocm),
      .mtxd_pad_o_from_the_ocm (top_mtxd_pad_o_from_the_ocm),
      .mtxen_pad_o_from_the_ocm (top_mtxen_pad_o_from_the_ocm),
      .mtxerr_pad_o_from_the_ocm (top_mtxerr_pad_o_from_the_ocm),
      .adsc_n_to_the_ssram (top_ssram_adsc_n),
      .bw_n_to_the_ssram (top_ssram_bw_n),
      .bwe_n_to_the_ssram (top_ssram_bwe_n),
      .chipenable1_n_to_the_ssram (top_ssram_ce_n),
      .clk (top_clkin_50),
      .ddr_sdram_aux_full_rate_clk_out (top_ddr_sdram_aux_full_rate_clk_out),
      .ddr_sdram_aux_half_rate_clk_out (top_ddr_sdram_aux_half_rate_clk_out),
      .ddr_sdram_phy_clk_out (top_ddr_sdram_phy_clk_out),
      .flash_ssram_tristate_bridge_address (top_flash_ssram_a),
      .flash_ssram_tristate_bridge_data (top_flash_ssram_d),
      .global_reset_n_to_the_ddr_sdram (top_reset_n),
      .in_port_to_the_button_pio (top_in_port_to_the_button_pio),
      .local_init_done_from_the_ddr_sdram (top_local_init_done_from_the_ddr_sdram),
      .local_refresh_ack_from_the_ddr_sdram (top_local_refresh_ack_from_the_ddr_sdram),
      .local_wdata_req_from_the_ddr_sdram (top_local_wdata_req_from_the_ddr_sdram),
      .mem_addr_from_the_ddr_sdram (top_mem_addr),
      .mem_ba_from_the_ddr_sdram (top_mem_ba),
      .mem_cas_n_from_the_ddr_sdram (top_mem_cas_n),
      .mem_cke_from_the_ddr_sdram (top_mem_cke),
      .mem_clk_n_to_and_from_the_ddr_sdram (top_mem_clk_n),
      .mem_clk_to_and_from_the_ddr_sdram (top_mem_clk),
      .mem_cs_n_from_the_ddr_sdram (top_mem_cs_n),
      .mem_dm_from_the_ddr_sdram (top_mem_dm),
      .mem_dq_to_and_from_the_ddr_sdram (top_mem_dq),
      .mem_dqs_to_and_from_the_ddr_sdram (top_mem_dqs),
      .mem_ras_n_from_the_ddr_sdram (top_mem_ras_n),
      .mem_we_n_from_the_ddr_sdram (top_mem_we_n),
      .out_port_from_the_led_pio (top_led),
      .outputenable_n_to_the_ssram (top_ssram_oe_n),
      .pll_c0_out (top_clk_to_offchip_video),
      .pll_c1_out (top_ssram_clk),
      .pll_c2_out (top_peripheral_clk),
      .pll_c3_out (top_remote_update_clk),
      .read_n_to_the_ext_flash (top_flash_oe_n),
      .reset_n (top_reset_n),
      .reset_phy_clk_n_from_the_ddr_sdram (top_reset_phy_clk_n_from_the_ddr_sdram),
      .select_n_to_the_ext_flash (top_flash_cs_n),
      .write_n_to_the_ext_flash (top_flash_wr_n)
    );

  // OpenCores Ethernet MAC signals
  assign top_HC_ETH_RESET_N = 1'b1;
  assign top_md_pad_i_to_the_ocm = top_mdio_pad_io_to_the_ocm;
  assign top_mdio_pad_io_to_the_ocm =  (top_md_padoe_o_from_the_ocm == 1'b1)? top_md_pad_o_from_the_ocm : 1'bz;           

  assign top_flash_reset_n = top_reset_n;
  assign top_in_port_to_the_button_pio = top_button;

endmodule

