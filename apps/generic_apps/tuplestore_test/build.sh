#!/bin/bash

EXP_DIR=experiments

mkdir -p $EXP_DIR

GIT_STATUS=$(git status -s |grep -v '^??'|grep -v build.sh|grep -v plot.py)
if [ ! -z "$GIT_STATUS" ]; then
	echo 'Git working dir not clean, you should commit if you want this to be repeatable'
	read
	#exit 1
fi

function generate_stuff() {
	# Area definitions
	#
	# when changing be sure to update gateway_to_db in plot.py
	case $AREA in
		alpha)
			FILENAME_GATEWAY=app_20140107091518.exe
			FILENAME_DB=app_ackto2000_r3_20131003082019.exe # ?
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
			FILENAME_GATEWAY=csma0_csmamode0_nullrdc_20131007122009.exe # ?
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

		echo)
			FILENAME_GATEWAY=csma0_csmamode0_contikimac16_T10k_20131011062327.exe
			FILENAME_DB=csma0_csmamode0_contikimac16_T10k_aon1_20131011173649.exe
			INODE_GATEWAY=inode019
			INODE_DB=inode018
			;;

		foxtrott)
			FILENAME_GATEWAY=csma0_csmamode0_contikimac8_T10k_20131012003722.exe
			FILENAME_DB=csma0_csmamode0_cxmac16_T10k_20131011062640.exe
			INODE_GATEWAY=inode017
			INODE_DB=inode020
			;;

		golf)
			FILENAME_GATEWAY=csma0_csmamode0_nullrdc_T10k_aon1_20131011155220.exe
			FILENAME_DB=csma0_csmamode0_nullrdc_T10k_aon1_comp1_20131029114826.exe
			INODE_GATEWAY=inode023
			INODE_DB=inode022
			;;

		hotel)
			FILENAME_GATEWAY=csma0_csmamode0_xmac16_T10k_20131011061706.exe
			FILENAME_DB=csma1_csmamode1_xmac8_T10k_20131008185822.exe
			INODE_GATEWAY=inode025
			INODE_DB=inode024
			;;

		india)
			FILENAME_GATEWAY=csma1_csmamode1_nullrdc_T10k_comp1_20131020144914.exe
			FILENAME_DB=csma1_csmamode1_contikimac16_T10k_aon1_20131011134329.exe
			INODE_GATEWAY=inode027
			INODE_DB=inode026
			;;

		julia)
			FILENAME_GATEWAY=csma1_csmamode1_contikimac8_T10k_20131009081631.exe
			FILENAME_DB=csma1_csmamode1_cxmac16_T10k_20131013063353.exe
			INODE_GATEWAY=inode029
			INODE_DB=inode028
			;;

		kilo)
			FILENAME_GATEWAY=csma1_csmamode1_cxmac8_T10k_20131008155100.exe
			FILENAME_DB=csma1_csmamode1_nullrdc_T10k_20131013133812.exe
			INODE_GATEWAY=inode031
			INODE_DB=inode030
			;;

			# unused:
#csma1_csmamode1_xmac16_T10k_20131012214526.exe
#csma1_csmamode1_xmac16_T10k_aon1_20131012212614.exe
#csma1_csmamode1_nullrdc_T10k_aon1_20131013182131.exe
#csma1_csmamode1_nullrdc_T10k_aon1_comp1_20131029092703.exe
# 

# 

		*)
			echo AREA unknown.
			exit 1
			;;
	esac

	GIT_COMMIT=$(git log --pretty=oneline -n 1|{ read A B; echo $A; } )
	GIT_MSG=$(git log --pretty=oneline -n 1|{ read A B; echo $B; } )
	GIT_DATE=$(git log --pretty='format:%cd' -n 1)

	if [ -e $EXP_DIR/current_exp_nr ]; then
		EXP_NR=$(< $EXP_DIR/current_exp_nr)
	fi
	if [ -z "$EXP_NR" ]; then
		EXP_NR=0
	fi
	EXP_NR=$(($EXP_NR + 1))
	echo $EXP_NR > $EXP_DIR/current_exp_nr
	if [ "$TS_DICT" == "tree" ]; then
		# incontextsensing
		#NTUPLES=53 
		#NTUPLES=43 
		NTUPLES=21
	elif [ "$TS_DICT" == "avl" ]; then
		NTUPLES=32
	else
		NTUPLES=$(wc -l ${RDF}|awk '{print $1}')
	fi


	#cp $RDF ${AREA}.rdf

	echo "#define APP_DATABASE_DEBUG $DEBUG" > defs.h
	echo "#define NTUPLES $NTUPLES" >> defs.h
	echo "#define EXP_NR $EXP_NR" >> defs.h
	echo "#define MODE \"$MODE\"" >> defs.h
	echo "#define MODE_$(echo $MODE|tr [a-z] [A-Z]) 1" >> defs.h
	if [ "$MODE" == "find" ]; then
		echo "#define APP_DATABASE_FIND 1" >> defs.h
	fi
	echo "#define DATASET \"$RDF\"" >> defs.h
	echo "#define DATABASE \"$DB\"" >> defs.h
	echo "#define GIT_COMMIT \"$GIT_COMMIT\"" >> defs.h
	echo "#define GIT_MSG \"$GIT_MSG\"" >> defs.h
	echo "#define GIT_DATE \"$GIT_DATE\"" >> defs.h
	echo "#define AREA \"$AREA\"" >> defs.h

	echo "#define TS_USE_$(echo $TS_DICT|tr [a-z] [A-Z])_DICT 1" >> defs.h
	echo "#define TS_CONTAINER_SIZE $TS_CONTAINER_SIZE" >> defs.h

	echo "#define TS_USE_$(echo $TS_CONTAINER|tr [a-z] [A-Z])_CONTAINER 1" >> defs.h
	echo "#define TS_DICT_SIZE $TS_DICT_SIZE" >> defs.h
	echo "#define TS_DICT_SLOTSIZE $TS_DICT_SLOTSIZE" >> defs.h

	echo EXP_NR=$EXP_NR > $EXP_DIR/${INODE_GATEWAY}.vars
	echo NTUPLES=$NTUPLES >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo DEBUG=$DEBUG >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo MODE=\"$MODE\" >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo AREA=\"$AREA\" >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo GIT_COMMIT=\"$GIT_COMMIT\" >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo GIT_DATE=\"$GIT_DATE\" >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo GIT_MSG=\"$GIT_MSG\" >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo DATASET=\"$RDF\" >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo DATABASE=\"$DB\" >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo GENERATED=\"$(date)\" >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo INODE_GATEWAY=\"${INODE_GATEWAY}\" >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo INODE_DB=\"${INODE_DB}\" >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo TS_CONTAINER=\"${TS_CONTAINER}\" >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo TS_CONTAINER_SIZE=${TS_CONTAINER_SIZE} >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo TS_DICT=\"${TS_DICT}\" >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo TS_DICT_SIZE=${TS_DICT_SIZE} >> $EXP_DIR/${INODE_GATEWAY}.vars
	echo TS_DICT_SLOTSIZE=${TS_DICT_SLOTSIZE} >> $EXP_DIR/${INODE_GATEWAY}.vars
	#cat ${INODE_GATEWAY}.vars

	cp $EXP_DIR/${INODE_GATEWAY}.vars $EXP_DIR/exp${EXP_NR}.vars


	make -f Makefile.gateway clean
	make -f Makefile.gateway || exit 1
	cp out/contiki-sky/app_gateway.exe $EXP_DIR/$FILENAME_GATEWAY || exit 1

	make -f Makefile.$DB clean
	if [ "$DB" == "teeny" ]; then
		make -f Makefile.teeny telosb || exit 1
		cp build/telosb/main.exe $EXP_DIR/$FILENAME_DB || exit 1
	else
		make -f Makefile.$DB || exit 1
		cp out/contiki-sky/app_database.exe $EXP_DIR/$FILENAME_DB || exit 1
	fi

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
DB=tuplestore
MODE=insert
TS_DICT=tree
#TS_CONTAINER=vector_static
TS_CONTAINER=set_static
TS_CONTAINER_SIZE=76
TS_DICT_SIZE=149
TS_DICT_SLOTSIZE=14

#DB=antelope
#DB=teeny

AREA=alpha
generate_stuff

AREA=bravo
generate_stuff

AREA=charlie
generate_stuff

AREA=delta
generate_stuff

AREA=echo
generate_stuff
AREA=foxtrott
generate_stuff
AREA=golf
generate_stuff
AREA=hotel
generate_stuff
AREA=india
generate_stuff
AREA=julia
generate_stuff
AREA=kilo
generate_stuff

ls -lh $EXP_DIR/*.exe
echo EXP_NR $EXP_NR

#./sync_ibbt.sh 


