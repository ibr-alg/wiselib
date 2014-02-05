#!/bin/bash

GIT_STATUS=$(git status -s |grep -v '^??'|grep -v build.sh|grep -v plot.py)
if [ ! -z "$GIT_STATUS" ]; then
	echo 'Git working dir not clean, you should commit if you want this to be repeatable'
	read
	#exit 1
fi

function generate_stuff() {
	# Area definitions
	case $AREA in
		alpha)
			FILENAME_GATEWAY=app_20140107091518.exe
			FILENAME_DB=app_ackto2000_r3_20131003082019.exe
			#INODE_GATEWAY=inode001
			#INODE_DB=inode004
			INODE_GATEWAY=inode015
			INODE_DB=inode016
			;;
		bravo)
			FILENAME_GATEWAY=example_app_20130920065808.exe
			FILENAME_DB=example_app_contiki_20130923150233.exe
			INODE_GATEWAY=inode007
			INODE_DB=inode010
			;;
		charlie)
			FILENAME_GATEWAY=csma0_csmamode0_nullrdc_20131007122009.exe
			FILENAME_DB=csma0_csmamode0_nullrdc_T10k_20131013061532.exe
			#INODE_GATEWAY=inode013
			#INODE_DB=inode012
			INODE_GATEWAY=inode011
			INODE_DB=inode014
			;;
		delta)
			FILENAME_GATEWAY=csma1_csmamode1_nullrdc_20131007080404.exe
			FILENAME_DB=csma1_csmode0_nullrdc_20131007102520.exe
			INODE_GATEWAY=inode009
			INODE_DB=inode008
			;;
		*)
			echo AREA unknown.
			exit 1
			;;
	esac

	GIT_COMMIT=$(git log --pretty=oneline -n 1|{ read A B; echo $A; } )
	GIT_MSG=$(git log --pretty=oneline -n 1|{ read A B; echo $B; } )
	GIT_DATE=$(git log --pretty='format:%cd' -n 1)

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
	echo "#define MODE \"$MODE\"" >> defs.h
	if [ "$MODE" == "find" ]; then
		echo "#define APP_DATABASE_FIND 1" >> defs.h
	fi
	echo "#define DATASET \"$RDF\"" >> defs.h
	echo "#define DATABASE \"$DB\"" >> defs.h
	echo "#define GIT_COMMIT \"$GIT_COMMIT\"" >> defs.h
	echo "#define GIT_MSG \"$GIT_MSG\"" >> defs.h
	echo "#define GIT_DATE \"$GIT_DATE\"" >> defs.h
	echo "#define AREA \"$AREA\"" >> defs.h

	echo EXP_NR=$EXP_NR > ${INODE_GATEWAY}.vars
	echo NTUPLES=$NTUPLES >> ${INODE_GATEWAY}.vars
	echo DEBUG=$DEBUG >> ${INODE_GATEWAY}.vars
	echo MODE=\"$MODE\" >> ${INODE_GATEWAY}.vars
	echo AREA=\"$AREA\" >> ${INODE_GATEWAY}.vars
	echo GIT_COMMIT=\"$GIT_COMMIT\" >> ${INODE_GATEWAY}.vars
	echo GIT_DATE=\"$GIT_DATE\" >> ${INODE_GATEWAY}.vars
	echo GIT_MSG=\"$GIT_MSG\" >> ${INODE_GATEWAY}.vars
	echo DATASET=\"$RDF\" >> ${INODE_GATEWAY}.vars
	echo DATABASE=\"$DB\" >> ${INODE_GATEWAY}.vars
	echo GENERATED=\"$(date)\" >> ${INODE_GATEWAY}.vars
	echo INODE_GATEWAY=\"${INODE_GATEWAY}\" >> ${INODE_GATEWAY}.vars
	echo INODE_DB=\"${INODE_DB}\" >> ${INODE_GATEWAY}.vars
	#cat ${INODE_GATEWAY}.vars

	cp ${INODE_GATEWAY}.vars exp${EXP_NR}.vars


	make -f Makefile.gateway clean
	make -f Makefile.gateway || exit 1
	cp out/contiki-sky/app_gateway.exe $FILENAME_GATEWAY || exit 1

	make -f Makefile.$DB clean
	make -f Makefile.$DB || exit 1
	cp out/contiki-sky/app_database.exe $FILENAME_DB || exit 1

	make -f Makefile.host clean
	make -f Makefile.host || exit 1

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

rm *.exe

DEBUG=0
RDF=incontextsensing.rdf
MODE=find
DB=antelope

AREA=alpha
generate_stuff

AREA=bravo
generate_stuff

AREA=charlie
generate_stuff

# Delta seems to have a broken xmem or something?
AREA=delta
generate_stuff

ls -lh *.exe
echo EXP_NR $EXP_NR

#./sync_ibbt.sh 


