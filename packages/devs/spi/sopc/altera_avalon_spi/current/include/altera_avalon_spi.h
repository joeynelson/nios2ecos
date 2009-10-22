#ifndef CYGONCE_DEVS_SPI_SOPC_H
#define CYGONCE_DEVS_SPI_SOPC_H

#include <pkgconf/hal.h>
#include <pkgconf/io_spi.h>
#include <cyg/io/spi.h>


typedef struct cyg_spi_sopc_bus_s
{
    // ---- Upper layer data ----
    cyg_spi_bus   spi_bus;                  // Upper layer SPI bus data

    // ---- Lower layer data ----
    cyg_drv_mutex_t   transfer_mx;          // Transfer mutex
    cyg_drv_cond_t    transfer_cond;        // Transfer condition
    cyg_bool          transfer_end;         // Transfer end flag
    cyg_vector_t	  interrupt_number;		// Interrupt number
    cyg_addrword_t    base;                 // Base Address of the SPI peripheral
    cyg_bool          cs_up;                // Chip Select up flag

} cyg_spi_sopc_bus_t;

typedef struct cyg_spi_sopc_device_s
{
    // ---- Upper layer data ----
    cyg_spi_device spi_device;  // Upper layer SPI device data

    // ---- Lower layer data (configurable) ----
    cyg_uint8  dev_num;         // Device number
    cyg_bool   init;            // Is device initialized
} cyg_spi_sopc_device_t;

externC cyg_spi_sopc_bus_t cyg_spi_sopc_bus0;

#endif //CYGONCE_DEVS_SPI_SOPC_H
