#!/bin/bash

if [ -z $1 ]; then
	echo "Usage: ./ast.sh <gaps.bed>"
	exit -1
fi

ls -a *.bam

gaps=$1

echo "
$asset/bin/ast_hic $gaps *.bam > hic.bed"
$asset/bin/ast_hic $gaps *.bam > hic.bed
