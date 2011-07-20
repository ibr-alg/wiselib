
cmake_minimum_required(VERSION 2.8)

# Set this to the base directory of the wiselib installation
set(WISELIB_PATH "../../../../")

# Define include directories here
set(APPLICATION_INCLUDE ".")

# ------------

set(CMAKE_C_FLAGS "-g -O0 -Wall -pipe")
set(CMAKE_CXX_FLAGS "-g -O0 -Wall -pipe")

message("Wiselib path: ${WISELIB_PATH}")
message("If that seems wrong, edit CMakeLists.txt")

include_directories(
	${WISELIB_PATH}/wiselib.stable
	${WISELIB_PATH}/wiselib.testing
	.
	${APPLICATION_INCLUDE}
)

set(LIBS rt)

add_definitions(-DPC -DDEBUG -DOSMODEL="PC")

macro(app name)
	add_executable(${name} main.cc ${ARGN})
	target_link_libraries(${name} ${LIBS})
endmacro(app)


# List all apps here
# Syntax: app(appname sourcefile1.cc sourcefile2.cc ...)

app(send_app send_app.cc)
app(receive_app receive_app.cc)
app(timer_app timer_app.cc)
app(demo_app demo.cc)


