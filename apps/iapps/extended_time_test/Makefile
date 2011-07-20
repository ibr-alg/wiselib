all: JN5121 JN5139R JN5139R1 shawn pacemate
	

JN5121:
	@echo "----- Building for JENNIC 5121 target ------"
	@make JENNIC_CHIP=JN5121  -s -f Makefile.jennic
	@echo "----- Done  ------"

JN5139R:
	@echo "----- Building for JENNIC 5139R target ------"
	@make JENNIC_CHIP=JN5139R -s -f Makefile.jennic
	@echo "----- Done  ------"

JN5139R1:
	@echo "----- Building for JENNIC 5139R1 target ------"
	@make JENNIC_CHIP=JN5139R1 -f Makefile.jennic
	@echo "----- Done  ------"

shawn:
	@echo "----- Building for SHAWN target  ------"
	@make -s -f Makefile.shawn  
	@echo "----- Done  ------"

pacemate:
	@echo "----- Building for PACEMATE target  ------"
	@make -s -f Makefile.pacemate
	@echo "----- Done  ------"
	
open-source:
	@echo "----- Building for OPEN-SOURCE target  ------"
	@make -s -f Makefile.open-source
	@echo "----- Done  ------"

msp430:
	@echo "----- Building MSP430 target ------"
	@make -f Makefile.msp430
	@echo "----- Done  ------"

clean: clean-JN5121 clean-JN5139R clean-JN5139R1 clean-shawn clean-pacemate
	
clean-JN5121:
	@make JENNIC_CHIP=JN5121 -s -f Makefile.jennic clean
	@echo "----- JN5121 cleaned ------"

clean-JN5139R:
	@make JENNIC_CHIP=JN5139R -s -f Makefile.jennic clean
	@echo "----- JN5139R cleaned ------"

clean-JN5139R1:
	@make JENNIC_CHIP=JN5139R1 -s -f Makefile.jennic clean
	@echo "----- JN5139R1 cleaned ------"

clean-shawn:
	@make -s -f Makefile.shawn clean
	@echo "----- shawn cleaned ------"
	
clean-pacemate:
	@make -s -f Makefile.pacemate clean
	@echo "----- pacemate cleaned ------"
	
clean-open-source:
	@make -s -f Makefile.open-source clean
	@echo "----- open-source clean  ------"
	
clean-msp430:
	@make -s -f Makefile.msp430 clean
	@echo "----- MSP430 cleaned ------"	
	
.PHONY: all buildbin debug buildlibs JN5121 JN5139R JN5139R1 shawn pacemate clean-JN5121 clean-JN5139R clean-JN5139R1 clean-shawn clean-pacemate msp430 clean-msp430