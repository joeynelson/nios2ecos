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

module ocm (
             // inputs:
              av_address,
              av_chipselect,
              av_clk,
              av_read,
              av_reset,
              av_rx_waitrequest,
              av_tx_readdata,
              av_tx_readdatavalid,
              av_tx_waitrequest,
              av_write,
              av_writedata,
              mcoll_pad_i,
              mcrs_pad_i,
              md_pad_i,
              mrx_clk_pad_i,
              mrxd_pad_i,
              mrxdv_pad_i,
              mrxerr_pad_i,
              mtx_clk_pad_i,

             // outputs:
              av_irq,
              av_readdata,
              av_rx_address,
              av_rx_byteenable,
              av_rx_write,
              av_rx_writedata,
              av_tx_address,
              av_tx_read,
              av_waitrequest_n,
              md_pad_o,
              md_padoe_o,
              mdc_pad_o,
              mtxd_pad_o,
              mtxen_pad_o,
              mtxerr_pad_o
           )
;

  output           av_irq;
  output  [ 31: 0] av_readdata;
  output  [ 31: 0] av_rx_address;
  output  [  3: 0] av_rx_byteenable;
  output           av_rx_write;
  output  [ 31: 0] av_rx_writedata;
  output  [ 31: 0] av_tx_address;
  output           av_tx_read;
  output           av_waitrequest_n;
  output           md_pad_o;
  output           md_padoe_o;
  output           mdc_pad_o;
  output  [  3: 0] mtxd_pad_o;
  output           mtxen_pad_o;
  output           mtxerr_pad_o;
  input   [  9: 0] av_address;
  input            av_chipselect;
  input            av_clk;
  input            av_read;
  input            av_reset;
  input            av_rx_waitrequest;
  input   [ 31: 0] av_tx_readdata;
  input            av_tx_readdatavalid;
  input            av_tx_waitrequest;
  input            av_write;
  input   [ 31: 0] av_writedata;
  input            mcoll_pad_i;
  input            mcrs_pad_i;
  input            md_pad_i;
  input            mrx_clk_pad_i;
  input   [  3: 0] mrxd_pad_i;
  input            mrxdv_pad_i;
  input            mrxerr_pad_i;
  input            mtx_clk_pad_i;

  wire             av_irq;
  wire    [ 31: 0] av_readdata;
  wire    [ 31: 0] av_rx_address;
  wire    [  3: 0] av_rx_byteenable;
  wire             av_rx_write;
  wire    [ 31: 0] av_rx_writedata;
  wire    [ 31: 0] av_tx_address;
  wire             av_tx_read;
  wire             av_waitrequest_n;
  wire             md_pad_o;
  wire             md_padoe_o;
  wire             mdc_pad_o;
  wire    [  3: 0] mtxd_pad_o;
  wire             mtxen_pad_o;
  wire             mtxerr_pad_o;
  eth_ocm the_eth_ocm
    (
      .av_address          (av_address),
      .av_chipselect       (av_chipselect),
      .av_clk              (av_clk),
      .av_irq              (av_irq),
      .av_read             (av_read),
      .av_readdata         (av_readdata),
      .av_reset            (av_reset),
      .av_rx_address       (av_rx_address),
      .av_rx_byteenable    (av_rx_byteenable),
      .av_rx_waitrequest   (av_rx_waitrequest),
      .av_rx_write         (av_rx_write),
      .av_rx_writedata     (av_rx_writedata),
      .av_tx_address       (av_tx_address),
      .av_tx_read          (av_tx_read),
      .av_tx_readdata      (av_tx_readdata),
      .av_tx_readdatavalid (av_tx_readdatavalid),
      .av_tx_waitrequest   (av_tx_waitrequest),
      .av_waitrequest_n    (av_waitrequest_n),
      .av_write            (av_write),
      .av_writedata        (av_writedata),
      .mcoll_pad_i         (mcoll_pad_i),
      .mcrs_pad_i          (mcrs_pad_i),
      .md_pad_i            (md_pad_i),
      .md_pad_o            (md_pad_o),
      .md_padoe_o          (md_padoe_o),
      .mdc_pad_o           (mdc_pad_o),
      .mrx_clk_pad_i       (mrx_clk_pad_i),
      .mrxd_pad_i          (mrxd_pad_i),
      .mrxdv_pad_i         (mrxdv_pad_i),
      .mrxerr_pad_i        (mrxerr_pad_i),
      .mtx_clk_pad_i       (mtx_clk_pad_i),
      .mtxd_pad_o          (mtxd_pad_o),
      .mtxen_pad_o         (mtxen_pad_o),
      .mtxerr_pad_o        (mtxerr_pad_o)
    );
  defparam the_eth_ocm.RX_FIFO_SIZE_IN_BYTES = 4096,
           the_eth_ocm.TOTAL_DESCRIPTORS = 128,
           the_eth_ocm.TX_FIFO_SIZE_IN_BYTES = 128;


endmodule

