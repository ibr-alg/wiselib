# shell script

# get the number of line having WISELIB_BASE variable in Makefile.local file
#   first, we use "=" to split the line and then use " " to further split the left content
wiselib_base_line=` echo "a" | cat ../generic_apps/Makefile.local | awk -F"=" '{print $1}' | awk -F" " '{if($2=="WISELIB_BASE") print NR}'`;
# retrive path from awk script and save it to WISELIB_BASE
WISELIB_BASE=` echo "b" | cat ../generic_apps/Makefile.local | awk -v line="$wiselib_base_line" -F" " '{if (NR== line ) print $2}' | awk -F"=" '{print $2}'`;

ns3_include_line=` echo "c" | cat ../generic_apps/Makefile.local | awk -F"=" '{print $1}' | awk -F" " '{if($2=="NS3_INCLUDE_DIR") print NR}'`;
NS3_INCLUDE_DIR=` echo "d" | cat ../generic_apps/Makefile.local | awk -v line="$ns3_include_line" -F" " '{if (NR== line ) print $2}' | awk -F"=" '{print $2}'`;

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

# run testing script in NS-3
#./waf --run src/wiselib/examples/wiselib-ns3-example
