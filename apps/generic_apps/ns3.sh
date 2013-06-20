#. ../generic_apps/Makefile.local

WISELIB_BASE=/home/dizhi/git-repo/wiselib/
NS3_INCLUDE_DIR=/home/dizhi/git-repo/ns-3-dev-git/

cd $NS3_INCLUDE_DIR
cd src/wiselib/examples

# refresh
cp wscript_templete wscript

# set paths
sed -i "s@WISELIB_PATH = ''@WISELIB_PATH = \"$WISELIB_BASE\"@" wscript
sed -i "s@NS3_PATH = ''@NS3_PATH = \"$NS3_INCLUDE_DIR\"@" wscript
cd ../../../

# configure and build NS-3 with examples
./waf configure --enable-examples
./waf
./waf --run src/wiselib/examples/wiselib-ns3-example
