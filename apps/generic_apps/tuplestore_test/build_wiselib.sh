#!/bin/bash

DB=antelope
RDF=incontextsensing.rdf

cp $RDF data.rdf
echo "#define NTUPLES $(wc -l data.rdf|awk '{print $1}')" > ntuples.h

make -f Makefile.gateway
make -f Makefile.$DB



