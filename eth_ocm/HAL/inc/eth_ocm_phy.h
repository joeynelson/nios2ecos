#ifndef __ETH_OCM_PHY_H__
#define __ETH_OCM_PHY_H__

#include "eth_ocm_regs.h"
#include "eth_ocm.h"

/* Enumeration of commonly-used PHY registers */
#define ETH_OCM_PHY_ADDR_CONTROL    0x0
#define ETH_OCM_PHY_ADDR_STATUS     0x1
#define ETH_OCM_PHY_ADDR_PHY_ID1    0x2
#define ETH_OCM_PHY_ADDR_PHY_ID2    0x3
#define ETH_OCM_PHY_ADDR_ADV        0x4
#define ETH_OCM_PHY_ADDR_PHY_REMADV 0x5

#define ETH_OCM_USE_INTERNAL_PHY_INIT

// Various PHY IDs
#define ETH_OCM_PHYID_LXT972A       0x001378E2  /* Intel LXT972A */
#define ETH_OCM_PHYID_MVL		    0x0141		/* Marvell 88E1111 */
#define ETH_OCM_PHYID_DP83848C      0x20005C90  /* National DP83848C */
#define ETH_OCM_PHYID_VCS8641       0x00070431  /* Vitesse 8641 Gigabit */

#define ETH_OCM_PHY_TIMEOUT_THRESHOLD   100000

void eth_ocm_set_phy_addr(int base, int phyad, int reg);
void eth_ocm_write_phy_reg(int base, int phyad, int reg, int data);
int  eth_ocm_read_phy_reg(int base, int phyad, int reg);

enum {
        PCS_CTL_speed1           = 1<<6,        // speed select
        PCS_CTL_speed0           = 1<<13,       
        PCS_CTL_fullduplex       = 1<<8,        // fullduplex mode select
        PCS_CTL_an_restart       = 1<<9,        // Autonegotiation restart command
        PCS_CTL_isolate          = 1<<10,       // isolate command
        PCS_CTL_powerdown        = 1<<11,       // powerdown command
        PCS_CTL_an_enable        = 1<<12,       // Autonegotiation enable
        PCS_CTL_rx_slpbk         = 1<<14,       // Serial Loopback enable
        PCS_CTL_sw_reset         = 1<<15        // perform soft reset
        
};

/** PCS Status Register Bits. IEEE 801.2 Clause 22.2.4.2
 */
enum {
        PCS_ST_has_extcap   = 1<<0,             // PHY has extended capabilities registers       
        PCS_ST_rx_sync      = 1<<2,             // RX is in sync (8B/10B codes o.k.)
        PCS_ST_an_ability   = 1<<3,             // PHY supports autonegotiation
        PCS_ST_rem_fault    = 1<<4,             // Autonegotiation completed
        PCS_ST_an_done      = 1<<5
        
};


#endif  //__ETH_OCM_PHY_H__
