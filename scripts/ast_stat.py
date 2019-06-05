#! /usr/bin/env python3
# Calculate absolute and relative assembly score
# N50, maximum, total, Ns
import sys
def col_evd(min_suprt, evd_fn, ctg_evd):
	tsc = 0
	tlen = 0
	prers = -1
	prere = -1
	prectg = ""
	ctge = -1
	with open(evd_fn) as f:
			for ln in f:
					lnlist = ln.strip().split('\t')
					ctgn = lnlist[0]
					rs = int(lnlist[1])
					re = int(lnlist[2])
					ntech = int(lnlist[3])
					if ctgn not in ctg_evd: # contig name is in order, this is a new contig
							if prers != -1:
									ctg_evd[prectg].append([prers, prere])
							if ctge != -1:
									tlen += ctge

							prers = -1
							prere = -1
							prectg = ctgn
							ctg_evd[ctgn] = []
					if ntech >= min_suprt:
							if rs > prere + 1:
									if prers != -1:
											ctg_evd[ctgn].append([prers, prere])
									prers = rs
									prere = re
							else:
									prere = re
					else:
							if prers != -1:
									ctg_evd[ctgn].append([prers, prere])
							prers = -1
							prere = -1

					tsc += (re - rs + 1) * ntech
					ctge = re
			if prers != -1:
					ctg_evd[prectg].append([prers, prere])
			if ctge != -1:
					tlen += ctge
			f.close()
							 
	return [tsc, tlen]

def stat(ctg_evd):
		# new list of value   
	ls = []
	for k in ctg_evd:
			for z in ctg_evd[k]:
					ls.append(z[1] - z[0] + 1)
	maxa = max(ls) 
	suma = sum(ls)
	lls = len(ls)
	ls.sort(reverse=True)
	suml = [sum(ls[0:z + 1]) for z in range(lls)] 
	idx = [-1] * 6
	for z in range(lls):
			for i in range(5, 11):
					if suml[z] >= i * suma / 10:
							if idx[i-5] == -1:
									idx[i-5] = z
	# print (ls) 
	v = [ls[e] for e in idx]
	return (suma, maxa, lls, idx, v)

def print_stat(absco, suma, lls, maxa, idx, v, ntech):
	print ("absolute assembly score: {0:.2f}".format(absco))
	print ("relative assembly score: {0:.2f}".format(absco/ntech)) 
	print ("sum: {0}, largest: {1}, average: {2:.2f}".format(suma, maxa, suma/lls)) 
	# print (v)
	for z in [50, 60, 70, 80, 90, 100]:
			print ("N{0}: {1}, L{0}: {2}".format(z, v[z//10-5], idx[z//10 - 5])) 

if __name__ == "__main__":
	if len(sys.argv) < 2:
			print ("acc_stat <acc.bed>")
			exit (1)
	min_suprt = 2
	acc_fn = sys.argv[1]
	ctg_evd = {}
	[tscore, tlen] =col_evd(min_suprt, acc_fn, ctg_evd)
	(suma, maxa, lls, idx, v) = stat(ctg_evd) 
	print_stat(tscore/tlen, suma, lls, maxa, idx, v, 4)    


