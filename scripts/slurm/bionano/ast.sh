#!/bin/bash

echo "Usage: ./ast.sh"

if [ ! -s input.fofn ]; then
	echo "input.fofn required. Exit."
	exit -1
fi

fln=`wc -l input.fofn | awk '{print $1}'`

if [ "$fln" -gt 2 ]; then
	echo ">2 bionano enzymes found."
	ls */bionano_*.bed > bed.list
	bed1=`sed -n 1p bed.list`
	for i in $(seq 2 $LEN);
	do
		bed2=`sed -n ${i}p bed.list`
		$asset/bin/union $bed1 $bed2 > bn$i.bed
		bed1=bn$i.bed
	done
	mv $bed1 bn.bed
	rm bn[1-$(($LEN-1))].bed
elif [ "$fln" -eq 2 ]; then
	$asset/bin/union */bionano_*.bed > bn.bed
elif [ "$fln" -eq 1 ]; then
	cp */bionano_*.bed bn.bed
fi
