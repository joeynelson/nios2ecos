set -e

echo "Run smoketest of environment"
. nios_env.sh
rm -rf /tmp/niossmoketest
mkdir /tmp/niossmoketest
cd /tmp/niossmoketest
nios2configgen --ptf=$NIOS_ECOS/../neek/neek.ptf --cpu=cpu
ecosconfig new nios2_neek default
ecosconfig tree
make -s

