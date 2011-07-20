#=============
# Shawn module configuration for cmake build system
#=============
set ( INCLUDE_PATH_WISELIB_STABLE CACHE PATH "Path to Wiselib Stable" )
set ( INCLUDE_PATH_WISELIB_TESTING CACHE PATH "Path to Wiselib Testing" )
set ( INCLUDE_PATH_WISELIB_INCUBATION CACHE PATH "Path to Wiselib Incubation" )
set ( INCLUDE_PATH_CLUSTERING CACHE PATH "Additional Include Path" )

include_directories ( ${INCLUDE_PATH_WISELIB_INCUBATION} )
include_directories ( ${INCLUDE_PATH_WISELIB_TESTING} )
include_directories ( ${INCLUDE_PATH_WISELIB_STABLE} )


include_directories ( ${INCLUDE_PATH_CLUSTERING} )
# Name of this module

   set ( moduleName WISELIBCLUSTERING )

# Default status (ON/OFF)

   set ( moduleStatus OFF )

# List of libraries needed by this module, seperated by white space

   set ( moduleLibs  )
