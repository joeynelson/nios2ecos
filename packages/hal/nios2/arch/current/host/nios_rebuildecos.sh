# This is how to build Nios eCos
echo Running nios2configgen...
# This will generate the .cdl file for your .ptf/Nios configuration
nios2configgen --ptf=xxx.ptf --cpu=cpu
# This is the NEEK board
ecosconfig new nios2_neek minimal
ecosconfig tree
make
