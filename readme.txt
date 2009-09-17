Patches for instructions most welcome! 

Smoketest of nios2ecos repository:
==================================
Install Quartus 9. nios2ecos comes w/a .ptf file you can use
for smoke testing the build.  

0. If you are running Cygwin, then you may have to run
"sh fixquartus90cygwin.sh" to fix problems with scripts
having Windows line endings, which the latest versions
of bash that ships with Cygwin is incompatible with.
This is necessary with Quartus 9.0 sp2.

1. Set up environment variables:

cd nios2ecos
. nios2_env.sh


2. Do a test build:

mkdir /tmp/ecos
cd /tmp/ecos
nios2configgen --ptf=/home/oyvind/workspace/nios2ecos/neek/neek.ptf --cpu=cpu
ecosconfig new nios2_neek default
ecosconfig tree
make
