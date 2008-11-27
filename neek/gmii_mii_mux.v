//  Ethernet Switch
// *************************************************************************
// Description: Mux the MII/GMII in combined 10/100/1000 PHY Interface and
//              implement the MII/GMII clock switch.
//
// Version    : $Id: gmii_mii_mux.vhd,v 1.1 2006/12/17 09:56:43 ttchong Exp $
// *************************************************************************


module gmii_mii_mux (

	reset_rx_clk,
	rx_clk,
	phy_rx_col,
	phy_rx_crs,
	phy_rx_d,
	phy_rx_dv,
	phy_rx_err,

	reset_tx_clk,
	tx_clk_ref125,  // 125 MHz TX local reference clock
	tx_clk_mac,     // on-chip TX clock to use by the MAC
	tx_clk,         // 2.5/25 MHz TX clock from PHY
	gtx_clk,        // GMII tx clock (wired from PLD_CLOCKOUT with clock buffers)
	phy_tx_d,
	phy_tx_en,
	phy_tx_err,

	mdio_in,        // to MAC
	mdio_out,
	mdio_oen,       
	mdio,           // wire MDC directly from MAC, therefore not visible here.

	// Local MII Interface

	m_rx_col,
	m_rx_crs,
	m_rx_d,
	m_rx_dv,
	m_rx_err,

	m_tx_d,
	m_tx_en,
	m_tx_err,

	// Local GMII Interface

	gm_rx_d,
	gm_rx_dv,
	gm_rx_err,

	gm_tx_d,
	gm_tx_en,
	gm_tx_err,

	eth_mode);         // from MAC configuration: 1: Gigabit, 0: 10/100


	// PHY Interface
					 
	input reset_rx_clk   ;
	input rx_clk         ;
	input phy_rx_col     ;
	input phy_rx_crs     ;
	input [7:0] phy_rx_d ;
	input phy_rx_dv      ;
	input phy_rx_err     ;
					 
	input reset_tx_clk   ;
	input tx_clk_ref125  ;        // 125 MHz TX local reference clock
	output tx_clk_mac    ;        // on-chip TX clock to use by the MAC
	input tx_clk         ;        // 2.5/25 MHz TX clock from PHY
	output gtx_clk       ;        // GMII tx clock (wired from PLD_CLOCKOUT with clock buffers)
	output [7:0] phy_tx_d;
	output phy_tx_en     ;
	output phy_tx_err    ;
					 
	output mdio_in       ;        // to MAC
	input mdio_out       ;
	input mdio_oen       ;       
	inout mdio           ;        // wire MDC directly from MAC, therefore not visible here.
					 
	// Local MII Interface
					 
	output m_rx_col      ;
	output m_rx_crs      ;
	output [3:0] m_rx_d  ;
	output m_rx_dv       ;
	output m_rx_err      ;

	input [3:0] m_tx_d   ;
	input m_tx_en        ;
	input m_tx_err       ;

	// Local GMII Interface

	output [7:0] gm_rx_d ;
	output gm_rx_dv      ;
	output gm_rx_err     ;

	input [7:0] gm_tx_d  ;
	input gm_tx_en       ;
	input gm_tx_err      ;

	input eth_mode       ;       // from MAC configuration: 1: Gigabit, 0: 10/100
	
	reg [7:0] phy_tx_d ;
	reg phy_tx_en     ;
	reg phy_tx_err    ;
	
	     // control 
        
         reg  mode_reg1;
         reg  mode_reg2;
        
         // RX input registers
         
         reg[7:0] gm_rx_d_r;
         reg gm_rx_dv_r;          
         reg gm_rx_err_r;
         
         wire tx_clk_int;     // TX clock Mux

         //attribute keep: boolean;
         //attribute keep of tx_clk_int: signal is true;
         
         wire gnd;
         wire vcc;
	

        // MDIO Tristate
        //--------------
        
        assign mdio_in = mdio;


        assign mdio =  (mdio_oen == 1'b0)? mdio_out : 1'bz;           

        

        // RX input regs to be moved into I/O
        //-----------------------------------

		always @(posedge reset_rx_clk or posedge rx_clk) 
		begin
		if(reset_rx_clk == 1'b1)
		begin
								gm_rx_d_r   <= 8'h00;
								gm_rx_dv_r  <= 1'b0;
								gm_rx_err_r <= 1'b0;

		end
		else
		begin
								gm_rx_d_r  <= phy_rx_d;
								gm_rx_dv_r <= phy_rx_dv;
								gm_rx_err_r<= phy_rx_err;

		end

		end
        

        
        assign m_rx_col  = phy_rx_col;
        assign m_rx_crs  = phy_rx_crs;
                  
        assign m_rx_d    = gm_rx_d_r[3 : 0];
        assign m_rx_dv   = gm_rx_dv_r;
        assign m_rx_err  = gm_rx_err_r;

        assign gm_rx_d   = gm_rx_d_r;
        assign gm_rx_dv  = gm_rx_dv_r;
        assign gm_rx_err = gm_rx_err_r;

        // TX Clock Mux
        //-------------

        always @(posedge reset_tx_clk or posedge tx_clk_ref125)
        begin
                if(reset_tx_clk==1'b1)
				begin
										
					mode_reg1 <= 1'b0;
					mode_reg2 <= 1'b0;
				end
				else
				begin

					// clock domain crossing

					mode_reg1 <= eth_mode;
					mode_reg2 <= mode_reg1;
                end
        
        end
             
 
        assign tx_clk_int = (mode_reg2== 1'b0)? tx_clk : tx_clk_ref125;

        assign tx_clk_mac = tx_clk_int;
        
        // Data Output Mux
        // ---------------

		always @(posedge reset_tx_clk or posedge tx_clk_int)
		begin
			if(reset_tx_clk==1'b1)
			begin
				  phy_tx_d   <= 8'h00;
				  phy_tx_en  <= 1'b0;
				  phy_tx_err <= 1'b0;
			end
			else
			begin
				if(mode_reg2==1'b1)
				begin
					  phy_tx_d  <= gm_tx_d;
					  phy_tx_en <= gm_tx_en;
					  phy_tx_err<= gm_tx_err;
				end
				else
				begin
					  phy_tx_d  <= {4'b0000, m_tx_d};
					  phy_tx_en <= m_tx_en;
					  phy_tx_err<= m_tx_err;
				end

			end
		end


        // GMII clock driver
        assign gnd = 1'b0;
        assign vcc = 1'b1;

        ddr_o phy_ckgen(
			.datain_h(vcc),
			.datain_l(gnd),
			.outclock(tx_clk_int),
			.dataout(gtx_clk));	

	
endmodule


