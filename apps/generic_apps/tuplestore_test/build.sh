#!/bin/bash


function generate_stuff() {
	# Area definitions
	case $AREA in
		alpha)
			FILENAME_GATEWAY=app_20140107091518.exe
			INODE_GATEWAY=inode001
			FILENAME_DB=app_ackto2000_r3_20131003082019.exe
			;;
		bravo)
			FILENAME_GATEWAY=example_app_20130920065808.exe
			INODE_GATEWAY=inode005
			FILENAME_DB=example_app_contiki_20130923150233.exe
			;;
		charlie)
			FILENAME_GATEWAY=csma1_csmamode1_nullrdc_20131007080404.exe
			INODE_GATEWAY=inode007
			FILENAME_DB=csma1_csmode0_nullrdc_20131007102520.exe
			;;
		delta)
			FILENAME_GATEWAY=csma0_csmamode0_nullrdc_20131007122009.exe
			INODE_GATEWAY=inode009
			FILENAME_DB=csma0_csmamode0_nullrdc_T10k_20131013061532.exe
			;;
		*)
			echo AREA unknown.
			exit 1
			;;
	esac

	GIT_STATUS=$(git status -s |grep -v '^??'|grep -v build.sh)
	if [ ! -z "$GIT_STATUS" ]; then
		echo 'Git working dir not clean, commit!'
		exit 1
	fi
	GIT_COMMIT=$(git log --pretty=oneline -n 1|{ read A B; echo $A; } )
	GIT_MSG=$(git log --pretty=oneline -n 1|{ read A B; echo $B; } )

	if [ -e current_exp_nr ]; then
		EXP_NR=$(< current_exp_nr)
	fi
	if [ -z "$EXP_NR" ]; then
		EXP_NR=0
	fi
	EXP_NR=$(($EXP_NR + 1))
	echo $EXP_NR > current_exp_nr

	#cp $RDF ${AREA}.rdf
	NTUPLES=$(wc -l ${RDF}|awk '{print $1}')

	echo "#define APP_DATABASE_DEBUG $DEBUG" > defs.h
	echo "#define NTUPLES $NTUPLES" >> defs.h
	echo "#define EXP_NR $EXP_NR" >> defs.h
	if [ "$MODE" == "find" ]; then
		echo "#define APP_DATABASE_FIND 1" >> defs.h
	fi
	echo "#define DATASET \"$RDF\"" >> defs.h
	echo "#define GIT_COMMIT \"$GIT_COMMIT\"" >> defs.h
	echo "#define GIT_MSG \"$GIT_MSG\"" >> defs.h
	echo "#define AREA \"$AREA\"" >> defs.h

	echo EXP_NR=$EXP_NR > ${INODE_GATEWAY}.vars
	echo NTUPLES=$NTUPLES >> ${INODE_GATEWAY}.vars
	echo DEBUG=$DEBUG >> ${INODE_GATEWAY}.vars
	echo MODE=\"$MODE\" >> ${INODE_GATEWAY}.vars
	echo AREA=\"$AREA\" >> ${INODE_GATEWAY}.vars
	echo GIT_COMMIT=\"$GIT_COMMIT\" >> ${INODE_GATEWAY}.vars
	echo GIT_MSG=\"$GIT_MSG\" >> ${INODE_GATEWAY}.vars
	echo DATASET=\"$RDF\" >> ${INODE_GATEWAY}.vars
	echo GENERATED=\"$(date)\" >> ${INODE_GATEWAY}.vars
	echo INODE_GATEWAY=\"${INODE_GATEWAY}\" >> ${INODE_GATEWAY}.vars

	cp ${INODE_GATEWAY}.vars exp${EXP_NR}.vars

	make -f Makefile.gateway && cp out/contiki-sky/app_database.exe $FILENAME_GATEWAY &&
	make -f Makefile.$DB && cp out/contiki-sky/app_database.exe $FILENAME_DB &&
	make -f Makefile.host

	echo
	echo '****************************'
	echo
	echo Experiment '#'$EXP_NR
	echo Database: $DB 
	echo Mode: $MODE
	echo Dataset: $RDF with $NTUPLES tuples
	echo
	echo '****************************'
	echo
}


AREA=alpha
DB=antelope
RDF=incontextsensing.rdf
MODE=find
DEBUG=0
generate_stuff

AREA=bravo
DB=tuplestore
RDF=incontextsensing.rdf
MODE=find
DEBUG=0
generate_stuff

AREA=charlie
DB=antelope
RDF=incontextsensing.rdf
MODE=insert
DEBUG=0
generate_stuff

AREA=delta
DB=tuplestore
RDF=incontextsensing.rdf
MODE=insert
DEBUG=0
generate_stuff

./sync_ibbt.sh 


