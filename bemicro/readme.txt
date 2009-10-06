This .sof+.ptf sample is for the Arrow/Hitex/Bemicro dongle.

http://www.arrownac.com/offers/altera-corporation/bemicro/


1. Build eCos & test cases:

sh build.sh

2. Configure sof:

nios2-configure-sof fpga.sof -r 

3. Run terminal

nios2-terminal

4. Start GDB server:

nios2-gdb-server --tcpport 1234 --tcppersist -r

5. Run test app:

nios2-elf-gdb install/tests/kernel/current/tests/tm_basic
target remote localhost:1234
load
continue

6. Admire eCos output

Regarding Linux: there are no Linux drivers for the dongle.
Using e.g. VirtualBox you can install the drivers. NB!
make sure to install programmer/setup.exe *before* you insert
the dongle and install drivers/*.inf afterwards.

Remember to use bridged networking in VirtualBox to be able to launch
GDB server
