#!/bin/sh

ROB=(32 64 128 256 512)
IQ=(8 16 32 64 128 256)
W=(1 2 4 8)
TRACE=(gcc perl)

for trace in ${TRACE[@]}; do
	for rob in ${ROB[@]}; do
		for iq in ${IQ[@]}; do
			for w in ${W[@]}; do
				echo "src/sim ${rob} ${iq} ${w} traces/val_trace_${trace}1 > output/re_${trace}_${rob}_${iq}_${w}.txt"
				src/sim ${rob} ${iq} ${w} traces/val_trace_${trace}1 > output/re_${trace}_${rob}_${iq}_${w}.txt
			done
		done
	done
done

