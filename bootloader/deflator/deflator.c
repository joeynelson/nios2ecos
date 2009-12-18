#include <pkgconf/system.h>     /* which packages are enabled/disabled */
#include <pkgconf/kernel.h>
#include <pkgconf/libc_startup.h>
#include <cyg/compress/zlib.h>
#include <cyg/hal/hal_arch.h>           /* CYGNUM_HAL_STACK_SIZE_TYPICAL */
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_diag.h>

#include <pkgconf/io_fileio.h>
#include <cyg/fileio/fileio.h>

#include <cyg/kernel/kapi.h>
#include <cyg/io/io.h>                  /* I/O functions */
#include <cyg/infra/diag.h>     // diag_printf

#include <cyg/compress/zlib.h>
#include <string.h>

#include "addresses.h"

cyg_uint8 inflateBuffer[8192];

#define NONCACHE_RAM_APPLICATION_START (0x80000000 | RAM_APPLICATION_START)
#define NONCACHE_ROM_APPLICATION_START (0x80000000 | ROM_APPLICATION_START)

int main(int argc, char **argv)
{
	cyg_interrupt_disable();
	diag_printf("Deflating application...\n");
	
	cyg_uint8 *DRAM = (cyg_uint8 *)NONCACHE_RAM_APPLICATION_START;

    z_stream d_stream; /* decompression stream */
    d_stream.zalloc = Z_NULL;
    d_stream.zfree = Z_NULL;
    d_stream.opaque = Z_NULL;

    d_stream.next_in  = NONCACHE_ROM_APPLICATION_START;
    d_stream.avail_in = 0x1000000;

    int err = inflateInit(&d_stream);

    if (err == Z_MEM_ERROR)
    {
    	diag_printf("Failed to deflate application %d\n", err);
    	HAL_PLATFORM_RESET();
    }

    int offset = 0;

    for (;;)
    {
        d_stream.avail_out = sizeof(inflateBuffer); 
        d_stream.next_out = inflateBuffer;

	    int err = inflate(&d_stream, Z_NO_FLUSH);
        int len = sizeof(inflateBuffer) - d_stream.avail_out;

        memcpy(DRAM + offset, inflateBuffer, len);
        
        offset += len;        
        
        if (err == Z_STREAM_END) 
        {
			diag_printf("Launching application(%d bytes)...\n", offset);
			/* launch application! */
			((void(*)(void))(NONCACHE_RAM_APPLICATION_START))();

			for (;;);
			// never reached
        } 

        if (err != Z_OK)
        {
        	diag_printf("Failed to deflate application %d\n", err);
        	return;
        	HAL_PLATFORM_RESET();
        }
    }
    diag_printf("Failed to deflate application %d\n", err);
	HAL_PLATFORM_RESET();

}
