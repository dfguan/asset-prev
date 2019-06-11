#! /bin/bash

if [ -z $1 ]; then
	echo "Usage: ./_submit_bionano.sh <ref.fasta>"
	echo "Assumes input.fofn in the path."
	exit -1
fi

input_fofn=input.fofn
LEN=`wc -l $input_fofn | awk '{print $1}'`

ref=$1
ref=`basename $ref`
ref=`echo $ref | sed 's/.fasta//g'`

cpus=36
mem=48g
name=bionano.map.$ref
script=$asset/scripts/slurm/bionano/map.sh
args="$1 $input_fofn"
partition=norm
walltime=2-0
path=`pwd`
extra="--array=1-$LEN"

mkdir -p logs
log=logs/$name.%A_%a.log

echo "\
sbatch -J $name --mem=$mem --partition=$partition --cpus-per-task=$cpus -D $path $extra --time=$walltime --error=$log --output=$log $script $args"
sbatch -J $name --mem=$mem --partition=$partition --cpus-per-task=$cpus -D $path $extra --time=$walltime --error=$log --output=$log $script $args > map_jid

dependency="--dependency=afterok:"`cat map_jid`

cpus=4
mem=8g
name=bionano.ast.$ref
script=$asset/scripts/slurm/bionano/ast.sh
args=""
partition=quick
walltime=4:00:00
path=`pwd`
log=logs/$name.%A.log

echo "\
sbatch -J $name --mem=$mem --partition=$partition --cpus-per-task=$cpus -D $path $dependency --time=$walltime --error=$log --output=$log $script $args"
sbatch -J $name --mem=$mem --partition=$partition --cpus-per-task=$cpus -D $path $dependency --time=$walltime --error=$log --output=$log $script $args > map_jid


