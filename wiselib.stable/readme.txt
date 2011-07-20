Introduction
*********************

wiselib.stable contains basically algorithms and data structures that
are thoroughly tested. It can thus be expected that the code runs on
any supported hardware and software platform without any hassles.



Folder Structure
*********************

The stable version of the Wiselib contains the following subfolders:
algorithms, external_interface, internal_interface, and util.


Algorithms
-------------------

The algorithms folder contains the implementations of the different
classes of algorithms. Each class has its own subfolder. In that
folder, the algorithm should be named
  ABBREVIATION_CLASS.h

That means, for example, a DSDV implementation (that belongs to the
class of algorithms) would be located and named in
  algorithms/routing/dsdv_routing.h
The clustering algorithm LEACH correspondingly
  algorithms/cluster/leach_cluster.h

If an algorithm belongs to more than one classes (e.g., a routing
algorithm that uses energy schemes), the algorithm should be added to
its /main class/.


External Interface
-------------------

The external interface contains the implementations of the Os
facets. Each class of facets in their own subfolder. So, when
implementing facets (radio and timer) for iSense, they would be
located in:
  external_interface/isense/isense_radio.h
  external_interface/isense/isense_timer.h

In addition, the header files of the implemenation must also be added
to the file
  external_interface/external_interface.h
by surrounding by an appropriate #ifdef. In the example above, when
adding radio and timer facet for iSense, it would look as follows:
  #ifdef ISENSE
  #include "external_interface/isense/isense_radio.h"
  #include "external_interface/isense/isense_timer.h"
  #endif


Internal Interface
-------------------

The internal interface contains implementations for the appropriate
concepts of data structures. Each implementation must be in a
subfolder named after the concept. For example, when we have two
implementations of a routing table (a /static/ one and one using
std::map from the STL), they would be named:
  internal_interface/routing_table/routing_table_static_array.h
  internal_interface/routing_table/routing_table_stl_map.h


Utils
-------------------

The util folder contains some useful implementation that can be used
by algorithms, data structures, and so on. For example, in the stable
version is a tiny STL called picoSTL (or pSTL for short) that provides
containers known from the STL with only static memory allocation. In
stable, there is also a serialization subfolder that contains code
that can be used for reading and writing data types (such as uint16_t,
own classes, ...) to buffers. This is especially useful when sending
messages over the radio interface in heterogeneous networks.
