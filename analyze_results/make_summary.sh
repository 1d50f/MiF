#!/bin/bash

for f in distrib/speck32_*; do
	echo $f; python analyze.py $f | tail -14;
done | tee summary_Speck32.txt

for f in distrib/speck64_*; do
	echo $f; python analyze.py $f | tail -14;
done | tee summary_Speck64.txt
