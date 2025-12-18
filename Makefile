.PHONY: all clean assembler simulator

all: assembler simulator

assembler:
	$(MAKE) -C Assembler

simulator:
	$(MAKE) -C Simulator

clean:
	$(MAKE) -C Assembler clean
	$(MAKE) -C Simulator clean
