Smoketest of nios2ecos repository:
==================================
Install Quartus 9 sp2 or newer. Older versions of
Quartus should work if you tweak nios2env.sh.

nios2ecos comes with a .sof and .ptf for NEEK you can
use for smoke testing the build procedure.

1. If you are running Cygwin, then you may have to run
"sh fixquartus90cygwin.sh" to fix problems with scripts
having Windows line endings, which the latest versions
of bash that ships with Cygwin is incompatible with.
This is necessary with Quartus 9.0 sp2.

2. Install ecos 3.0 in ~/ecos

3. Set up environment variables:

cd nios2ecos
. nios2_env.sh

4. Do a test build:

mkdir /tmp/ecos
cd /tmp/ecos
nios2configgen --ptf=$NIOS_ECOS/../neek/neek.ptf --cpu=cpu
ecosconfig new nios2_neek default
ecosconfig tree
make -s
