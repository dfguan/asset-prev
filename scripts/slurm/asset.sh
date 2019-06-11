#!/bin/bash

if [[ -z $1 ]] || [[ -z $2 ]]; then
    echo "Usage: ./_submit_asset.sh <asm.fasta> <name>"
    exit -1
fi

export asset=/data/Phillippy/tools/asset

path=`pwd`
path=${path/assembly_curated/genomic_data}

mkdir -p asset
cd asset

name=$2

if [ ! -e $name.fasta ]; then
    ln -s ../$1 $name.fasta
fi

echo "
$asset/bin/detgaps $name.fasta > gaps.bed"
$asset/bin/detgaps $name.fasta > gaps.bed

echo "Submit pacbio jobs"
mkdir -p pacbio
cd pacbio
ls $path/pacbio/fasta/*.fasta.gz > input.fofn
if [ ! -e $name.fasta ]; then
	ln -s ../$name.fasta
fi
sh $asset/scripts/slurm/minimap2/_submit_minimap2.sh $name.fasta
echo

cd ../

echo "Submit 10x jobs"
mkdir -p 10x
cd 10x
ls $path/10x/*R1_001.fastq.gz > R1.fofn
ls $path/10x/*R2_001.fastq.gz | paste R1.fofn - > input.fofn
rm R1.fofn
if [ ! -e gaps.bed ]; then
	ln -s ../gaps.bed
fi
sh $asset/scripts/slurm/bwa/_submit_bwa.sh ../$name.fasta
idx_jid=`cat idx_jid`
echo

cd ../

echo "Submit bionano jobs"
mkdir -p bionano
cd bionano
ls $path/bionano/*.cmap > input.fofn
if [ ! -e gaps.bed ]; then
	ln -s ../gaps.bed
fi
sh $asset/scripts/slurm/bionano/_submit_bionano.sh ../$name.fasta
echo

cd ../

echo "Submit hic jobs"
mkdir -p hic
cd hic
ls $path/hic/*R1.fastq.gz > R1.fofn
ls $path/hic/*R2.fastq.gz | paste R1.fofn - > input.fofn
rm R1.fofn
if [ ! -e gaps.bed ]; then
	ln -s ../gaps.bed
fi
sh $asset/scripts/slurm/hic/_submit_bwa_hic.sh ../$name.fasta $idx_jid


