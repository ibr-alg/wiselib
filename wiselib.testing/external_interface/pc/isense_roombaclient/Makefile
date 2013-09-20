all: serial

serial:
	@echo "----- Building Serial Roomba Application ------"
	@$(MAKE) TARGET=serial -f Makefile.build 

clean:
	@echo "----- Cleaning Applications ------"
	@$(MAKE) TARGET=serial --no-print-directory -f Makefile.build clean

clean-dep:
	@echo "----- Cleaning Dependencies ------"
	@$(MAKE) TARGET=serial -f Makefile.build --no-print-directory clean-dep

.PHONY: all roomba clean clean-dep
JN5148:
	@echo "----- Building Serial Roomba Application ------"
	@$(MAKE) TARGET=serial -f Makefile.build JN5148