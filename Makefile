# Root Makefile to call src make and tool make

all:
	$(MAKE) -C src all

sim:
	$(MAKE) -C src sim

test:
	@src/sim 256 32 4 gcc_trace.txt

run:
	@mkdir -p output
	src/sim 16 8 1 traces/val_trace_gcc1 > output/val1_16_8_1_gcc1.txt
	src/sim 16 8 2 traces/val_trace_gcc1 > output/val2_16_8_2_gcc1.txt
	src/sim 60 15 3 traces/val_trace_gcc1 > output/val3_60_15_3_gcc1.txt
	src/sim 64 16 8 traces/val_trace_gcc1 > output/val4_64_16_8_gcc1.txt
	src/sim 64 16 4 traces/val_trace_perl1 > output/val5_64_16_4_perl1.txt
	src/sim 128 16 5 traces/val_trace_perl1 > output/val6_128_16_5_perl1.txt
	src/sim 256 64 5 traces/val_trace_perl1 > output/val7_256_64_5_perl1.txt
	src/sim 512 64 7 traces/val_trace_perl1 > output/val8_512_64_7_perl1.txt
	@echo "gvimdiff output/val1_16_8_1_gcc1.txt val/val1.txt"
	@echo "gvimdiff output/val2_16_8_2_gcc1.txt val/val2.txt"
	@echo "gvimdiff output/val3_60_15_3_gcc1.txt val/val3.txt"
	@echo "gvimdiff output/val4_64_16_8_gcc1.txt val/val4.txt"
	@echo "gvimdiff output/val5_64_16_4_perl1.txt val/val5.txt"
	@echo "gvimdiff output/val6_128_16_5_perl1.txt val/val6.txt"
	@echo "gvimdiff output/val7_256_64_5_perl1.txt val/val7.txt"
	@echo "gvimdiff output/val8_512_64_7_perl1.txt val/val8.txt"
	-diff -iw output/val1_16_8_1_gcc1.txt val/val1.txt
	-diff -iw output/val2_16_8_2_gcc1.txt val/val2.txt
	-diff -iw output/val3_60_15_3_gcc1.txt val/val3.txt
	-diff -iw output/val4_64_16_8_gcc1.txt val/val4.txt
	-diff -iw output/val5_64_16_4_perl1.txt val/val5.txt
	-diff -iw output/val6_128_16_5_perl1.txt val/val6.txt
	-diff -iw output/val7_256_64_5_perl1.txt val/val7.txt
	-diff -iw output/val8_512_64_7_perl1.txt val/val8.txt

report: parse

parse:
	echo trace,rob,iq,w,ipc > re.csv
	python3 parse.py

exp: sim
	@mkdir -p output
	./run.sh

pack:
	rm project-3-nraman2.zip
	zip -j project-3-nraman2.zip src/Makefile src/*.cc src/*.h

clean:
	$(MAKE) -C src clean

clobber:
	$(MAKE) -C src clobber

cleanout:
	rm -f output/*.txt ; $(MAKE) -C src clean
