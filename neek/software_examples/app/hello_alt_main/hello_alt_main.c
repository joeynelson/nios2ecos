/* Copyright (c) 2004 Altera Corporation, San Jose, California, USA. All rights 
 * reserved. All use of this software and documentation is subject to the License
 * Agreement located at the end of this file below.
 ******************************************************************************
 *  DANGER ** WARNING ** Please read before proceeding! ** WARNING ** DANGER  *           
 ******************************************************************************
 *
 * "Hello World Free-Standing" (hello_alt_main) example. 
 *
 * This program is an example of a "free-standing" C application since it 
 * calls "alt_main()" instead of "main()". The example's purpose is to 
 * illustrate to the advanced embedded developer the low-level system 
 * initialization steps needed for a "Hello World" type application. By 
 * calling "alt_main()" instead of "main()", these system initialization steps 
 * are NOT linked in automatically, and must be provided by the developer, 
 * which this example illustrates. 
 *
 * Please refer to file readme.txt for notes on this software example.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "sys/alt_sys_init.h"
#include "sys/alt_irq.h"
#include "priv/alt_file.h"

/*
 * The following statement defines "main()" so that when the Nios II IDE 
 * debugger is set to break at "main()", it will break at the appropriate
 * place in this program, which does not contain a function called "main()".
 * Note that the Nios II IDE debugger can also be set to break at "alt_main()",
 * in which case the following statement would be unneccessary since 
 * "alt_main()" is defined in this program.
 */
int main (void) __attribute__ ((weak, alias ("alt_main")));


/*
 * _do_ctors() is used to call the C++ constructors. 
 * 
 * It is declared weak so that we don't have to force 
 * inclusion of _do_ctors if there are no C++ constructors
 * to call.
 *
 * Commended out here because this example is not a C++ program:
 */
//extern void _do_ctors(void) ALT_WEAK;


int alt_main(void)
{ 
  /* 
   * Enable Interrutps
   *
   * Turn on interrupts, and initialize the low-level interrupt handler:
   */
  alt_irq_init (ALT_IRQ_BASE);
 
  
  /* 
   * Device-Driver Initialization
   *
   * Initialize all the device drivers for every piece of hardware
   * in the current system.  
   *
   * Note that the function "alt_sys_init()" is defined in the 
   * AUTOMATICALLY-GENERATED file alt_sys_init.c. This file is 
   * generated "on the fly" into your System Library project 
   * at library-build time (as part of the Make-process). You can find the
   * file here:
   *    <system library name>/Release/system_description/alt_sys_init.c 
   * 
   * Because it is generated on-the-fly, your library's copy of 
   * alt_sys_init() is customized to initialize *only* the devices
   * in your *particuilar* Nios II system.  
   * 
   * Being machine-generated, alt_sys_init() initializes every single
   * device in your system--even ones which your application may not 
   * use. Indeed, this "Hello World" program only uses one--the
   * STDOUT device. If you wish to save code-space, you may want 
   * to initialize only the devices you actually need and use.
   *
   * To do so, you should define your own function (e.g. small_sys_init()). 
   * DO NOT edit the file alt_sys_init.c! If you do, your changes will be 
   * overwritten the next time you build the library! Please refer to the 
   * "readme.html" file that accompanies this software example to see how 
   * such a customized sys_init routine would look for UART initialization ONLY.
   */
   alt_sys_init();


  /* 
   * I/O Stream Initialization.
   * 
   * Initialize the STDOUT stream, and associate it with the 
   * System Library's designated STDOUT device.  Note that the symbols
   * ALT_STDOUT (etc) are defined in the (generated) file system.h. 
   *
   */
  alt_io_redirect (ALT_STDOUT, ALT_STDIN, ALT_STDERR);


  /* 
   * C++ Constructors.
   * 
   * This particular example is not a C++ program.  But, if it were, 
   * you would need to call all your static constructors as part of 
   * the initialization process.  To do so, you would un-comment this
   * line:
   *
   */
   //_do_ctors();


  printf("Hello from Nios II Free-Standing!\n");

  /* 
   * Exit gracefully.
   *
   * Many embedded programs run as long as the machine is powered-up.
   * Those programs don't need to call exit().  But even this humble
   * little "Hello World" application (which, indeed, does terminate)
   * needs to call exit().  The exit() function, amongst many other
   * things, flushes the I/O buffers--a singularly-important service
   * for this example:
   */
  exit(0);  // Return code for "success!" if anyone is checking (they aren't).
}

/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *                                                                 *
******************************************************************************/
