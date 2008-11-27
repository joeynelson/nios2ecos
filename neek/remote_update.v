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

module remote_update (
                       // inputs:
                        address,
                        chipselect,
                        clk,
                        read,
                        reset,
                        write,
                        writedata,

                       // outputs:
                        readdata,
                        waitrequest
                     )
;

  output  [ 31: 0] readdata;
  output           waitrequest;
  input   [  5: 0] address;
  input            chipselect;
  input            clk;
  input            read;
  input            reset;
  input            write;
  input   [ 31: 0] writedata;

  wire    [ 31: 0] readdata;
  wire             waitrequest;
  altera_avalon_remote_update_cycloneiii the_altera_avalon_remote_update_cycloneiii
    (
      .address     (address),
      .chipselect  (chipselect),
      .clk         (clk),
      .read        (read),
      .readdata    (readdata),
      .reset       (reset),
      .waitrequest (waitrequest),
      .write       (write),
      .writedata   (writedata)
    );


endmodule

