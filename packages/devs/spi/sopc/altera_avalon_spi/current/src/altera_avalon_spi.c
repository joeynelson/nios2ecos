#include <cyg/error/codes.h>

#include <pkgconf/hal.h>
#include <pkgconf/io_spi.h>
#include <cyg/io/altera_avalon_spi.h>
#include <cyg/hal/system.h>
#include <altera_avalon_spi_regs.h>

static void spi_sopc_init_bus(cyg_spi_sopc_bus_t * bus);
static void spi_sopc_transaction_begin(cyg_spi_device *dev);

static void spi_sopc_transaction_transfer(cyg_spi_device  *dev,
                                          cyg_bool         polled,
                                          cyg_uint32       count,
                                          const cyg_uint8 *tx_data,
                                          cyg_uint8       *rx_data,
                                          cyg_bool         drop_cs);

static void spi_sopc_transaction_tick(cyg_spi_device *dev,
                                      cyg_bool        polled,
                                      cyg_uint32      count);

static void spi_sopc_transaction_end(cyg_spi_device* dev);

static int spi_sopc_get_config(cyg_spi_device *dev,
                               cyg_uint32      key,
                               void           *buf,
                               cyg_uint32     *len);

static int spi_sopc_set_config(cyg_spi_device *dev,
                               cyg_uint32      key,
                               const void     *buf,
                               cyg_uint32     *len);

cyg_spi_sopc_bus_t cyg_spi_sopc_bus0 =
{
    .spi_bus.spi_transaction_begin    = spi_sopc_transaction_begin,
    .spi_bus.spi_transaction_transfer = spi_sopc_transaction_transfer,
    .spi_bus.spi_transaction_tick     = spi_sopc_transaction_tick,
    .spi_bus.spi_transaction_end      = spi_sopc_transaction_end,
    .spi_bus.spi_get_config           = spi_sopc_get_config,
    .spi_bus.spi_set_config           = spi_sopc_set_config,
    .interrupt_number                 = CMOS_SPI_IRQ,
    .base                             = CMOS_SPI_BASE
};





CYG_SPI_DEFINE_BUS_TABLE(cyg_spi_sopc_device_t, 1);

#ifndef CYGBLD_ATTRIB_C_INIT_PRI
# define CYGBLD_ATTRIB_C_INIT_PRI(x)
#endif

void CYGBLD_ATTRIB_C_INIT_PRI(CYG_INIT_BUS_SPI)
cyg_spi_sopc_bus_init(void)
{
   spi_sopc_init_bus(&cyg_spi_sopc_bus0);
}

static void spi_sopc_init_bus(cyg_spi_sopc_bus_t *bus)
{
	// Call upper layer bus init
	CYG_SPI_BUS_COMMON_INIT(&bus->spi_bus);
}

static void
spi_sopc_start_transfer(cyg_spi_sopc_device_t *dev)
{
    cyg_spi_sopc_bus_t *spi_bus = (cyg_spi_sopc_bus_t *)dev->spi_device.spi_bus;

    if (spi_bus->cs_up)
        return;

    // if needed, add a delay before setting the chip select

    // Raise CS
    IOWR_ALTERA_AVALON_SPI_SLAVE_SEL((spi_bus->base), 1 << (dev->dev_num));
    IOWR_ALTERA_AVALON_SPI_CONTROL((spi_bus->base), ALTERA_AVALON_SPI_CONTROL_SSO_MSK);

    // If needed, add a delay after setting the chip select

    spi_bus->cs_up = true;
}

static void
spi_sopc_drop_cs(cyg_spi_sopc_device_t *dev)
{
    cyg_spi_sopc_bus_t *spi_bus = (cyg_spi_sopc_bus_t *)dev->spi_device.spi_bus;

    if (!spi_bus->cs_up)
       return;

    //add a delay if needed

    // Drop CS
    IOWR_ALTERA_AVALON_SPI_SLAVE_SEL((spi_bus->base), 0);
    IOWR_ALTERA_AVALON_SPI_CONTROL((spi_bus->base), 0);

    spi_bus->cs_up = false;
}

static void
spi_sopc_transfer_polled(cyg_spi_sopc_device_t *dev,
                         cyg_uint32             count,
                         const cyg_uint8       *tx_data,
                         cyg_uint8             *rx_data)
{
    cyg_uint32 val, status;
    cyg_spi_sopc_bus_t *spi_bus = (cyg_spi_sopc_bus_t *)dev->spi_device.spi_bus;

    /*
     * Discard any stale data present in the RXDATA register, in case
     * previous communication was interrupted and stale data was left
     * behind.
     */
    IORD_ALTERA_AVALON_SPI_RXDATA((spi_bus->base));

    // Transmit and receive byte by byte
    while (count-- > 0)
    {
        do
        {
          status = IORD_ALTERA_AVALON_SPI_STATUS((spi_bus->base));
        }
        while ((status & ALTERA_AVALON_SPI_STATUS_TRDY_MSK) == 0);

        // Send next byte over the wire
        IOWR_ALTERA_AVALON_SPI_TXDATA((spi_bus->base), *tx_data++);
        /* Wait until the interface has finished transmitting */
        do
        {
          status = IORD_ALTERA_AVALON_SPI_STATUS((spi_bus->base));
        }
        while ((status & ALTERA_AVALON_SPI_STATUS_TMT_MSK) == 0);


        do
		{
			status = IORD_ALTERA_AVALON_SPI_STATUS((spi_bus->base));
		} while ((status & ALTERA_AVALON_SPI_STATUS_RRDY_MSK) == 0);

        //read and store only if rx_data is valid
        val = (alt_u8)IORD_ALTERA_AVALON_SPI_RXDATA((spi_bus->base));
        if (NULL != rx_data)
        {
            *rx_data++ = val;
        }
    }
}

// -------------------------------------------------------------------------

static void
spi_sopc_transaction_begin(cyg_spi_device *dev)
{
    cyg_spi_sopc_device_t *sopc_spi_dev = (cyg_spi_sopc_device_t *) dev;
    cyg_spi_sopc_bus_t *spi_bus =
      (cyg_spi_sopc_bus_t *)sopc_spi_dev->spi_device.spi_bus;
    cyg_uint32 val;

    if (!sopc_spi_dev->init)
    {
        sopc_spi_dev->init = true;
    }
}

static void
spi_sopc_transaction_transfer(cyg_spi_device  *dev,
                              cyg_bool         polled,
                              cyg_uint32       count,
                              const cyg_uint8 *tx_data,
                              cyg_uint8       *rx_data,
                              cyg_bool         drop_cs)
{
    cyg_spi_sopc_device_t *sopc_spi_dev = (cyg_spi_sopc_device_t *) dev;

    // Select the device if not already selected
    spi_sopc_start_transfer(sopc_spi_dev);

    // Perform the transfer
    //support only polled mode
	spi_sopc_transfer_polled(sopc_spi_dev, count, tx_data, rx_data);

    // Deselect the device if requested
    if (drop_cs)
        spi_sopc_drop_cs(sopc_spi_dev);
}

static void
spi_sopc_transaction_tick(cyg_spi_device *dev,
                          cyg_bool        polled,
                          cyg_uint32      count)
{
    const cyg_uint32 zeros[10] = { 0,0,0,0,0,0,0,0,0,0 };

    cyg_spi_sopc_device_t *sopc_spi_dev = (cyg_spi_sopc_device_t *) dev;

    // Transfer count zeros to the device - we don't touch the
    // chip select, the device could be selected or deselected.
    // It is up to the device driver to decide in wich state the
    // device will be ticked.

    while (count > 0)
    {
        int tcnt = count > 40 ? 40 : count;

        //support only polled
		spi_sopc_transfer_polled(sopc_spi_dev, tcnt,
								 (const cyg_uint8 *) zeros, NULL);
        count -= tcnt;
    }
}

static void
spi_sopc_transaction_end(cyg_spi_device* dev)
{
    cyg_spi_sopc_device_t * sopc_spi_dev = (cyg_spi_sopc_device_t *)dev;
    cyg_spi_sopc_bus_t *spi_bus =
      (cyg_spi_sopc_bus_t *)sopc_spi_dev->spi_device.spi_bus;

    spi_sopc_drop_cs((cyg_spi_sopc_device_t *) dev);
}

static int
spi_sopc_get_config(cyg_spi_device *dev,
                    cyg_uint32      key,
                    void           *buf,
                    cyg_uint32     *len)
{
    cyg_spi_sopc_device_t *sopc_spi_dev = (cyg_spi_sopc_device_t *) dev;

    switch (key)
    {
        case CYG_IO_GET_CONFIG_SPI_CLOCKRATE:
        {
            if (*len != sizeof(cyg_uint32))
                return -EINVAL;
            else
            {
//                cyg_uint32 *cl_brate = (cyg_uint32 *)buf;
//                *cl_brate = sopc_spi_dev->cl_brate;
            }
        }
        break;
        default:
            return -EINVAL;
    }
    return ENOERR;
}

static int
spi_sopc_set_config(cyg_spi_device *dev,
                    cyg_uint32      key,
                    const void     *buf,
                    cyg_uint32     *len)
{
    cyg_spi_sopc_device_t *sopc_spi_dev = (cyg_spi_sopc_device_t *) dev;

    switch (key)
    {
        case CYG_IO_SET_CONFIG_SPI_CLOCKRATE:
        {
            if (*len != sizeof(cyg_uint32))
                return -EINVAL;
            else
            {
//                cyg_uint32 cl_brate     = *((cyg_uint32 *)buf);
//                cyg_uint32 old_cl_brate = sopc_spi_dev->cl_brate;
//
//                sopc_spi_dev->cl_brate = cl_brate;
//
//                if (!spi_sopc_calc_scbr(sopc_spi_dev))
//                {
//                    sopc_spi_dev->cl_brate = old_cl_brate;
//                    spi_sopc_calc_scbr(sopc_spi_dev);
//                    return -EINVAL;
//                }
            }
        }
        break;
        default:
            return -EINVAL;
    }
    return ENOERR;
}
