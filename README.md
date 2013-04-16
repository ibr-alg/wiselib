Introduction
=====================

The Wiselib is an algorithm library for sensor networks. It contains
various algorithm classes (for instance, localization or routing) that
can be compiled for several sensor network platforms such as iSense or
Contiki, or the sensor network simulator Shawn. It is completely
written in C++, and uses templates in the same way as Boost and
CGAL. This makes it possible to write generic and platform independent
code that is very efficiently compiled for the various platforms.

We provide easy-to-use interfaces to the OS, which simplifies the
development process and decreases the need for dealing with low-level
functionality of specific hardware platforms. Algorithms can even be
run in a simulation environment first (for debugging purposes, for
example), and then compiled for real hardware platforms without
changing any line of algorithm code.

Algorithms can be directly integrated in your application. For
example, when developing an application for iSense that collects
sensor data values, a Wiselib routing algorithm can be used to route
the data to a sink. Another scenario are pure Wiselib applications: We
provide an own application_main that is called by Wiselib code, and
you can integrate algorithm implementations there. The advantage is
that these applications can be compiled for any supported platform,
without changing any line of code (instead, just makefile
targets). Similarly to the own Wiselib application, any algorithm can
be compiled as an OpenCom? component, which allows for dynamically
loading an algorithm in your application.


Folder Structure
=====================

There are basically three kinds of folders. First, the Wiselib code in
wiselib.stable and wiselib.testing (which only contains algorithms,
but no stand-alone applications). Second, examples of how to integrate
Wiselib algorithms in own applications (applications, iapps,
shawn_apps). Third, the sandbox with some test code (e.g., comparison
of virtual inheritence with pure template solutions).


wiselib.stable
-------------------

Contains stable Wiselib code that was tested for all supported
platforms (at least, for all platforms that are part of stable). The
code is divided into algorithms (the heart of the Wiselib), external
interfaces (connections to the different OSs), internal interfaces
(data structures), and util (different useful things such as data
serialization, the pSTL, or delegates for efficient callbacl
realization).

wiselib.testing
-------------------

Contains newly developed Wiselib code that may not be tested on all
platforms, or implementations of concepts that may change in the near
future. The structure is the same as in wiselib.stable: algorithms,
external interfaces, internal interfaces, and util.

apps
-------------------

apps/generic_apps
.................

Contains examples of standalone Wiselib applications with an own
application_main method. These apps can be used to write a whole
application once, and compile it for different platforms just by
changing the make target.

iapps
...................

Examples of Wiselib integration in native iSense applications.

shawn_apps
...................

Examples of Wiselib integration in Shawn processors. Note that the
Wiselib module in shawn.svn/src/apps must be enabled!

pc_apps
...................

Run Wiselib application on PC - examples for ordinary appplications
(with an iSense node attached to the PC, the PC can participate in a
iSense sensor network!), and apps where a netbook is attached to a
Roomba.

doc
-------------------

Main sources for `doxygen` documentation. Contains concept descriptions
and main doxygen pages. To create doygen documentation, just run:
 doxygen Doxyfile
in Wiselib's root folder.
