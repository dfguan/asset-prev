#!/bin/bash

if [ -z $1 ]; then
	echo "Usage: ast.sh <out_prefix>"
	exit -1
fi

prefix=$1

export asset=/data/Phillippy/tools/asset

echo "
$asset/bin/ast_pb *.paf > pb.bed"
$asset/bin/ast_pb *.paf > pb.bed &&

rm *.paf || echo "ERROR"


