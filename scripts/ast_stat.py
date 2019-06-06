#! /usr/bin/env python3
# Calculate absolute and relative assembly score
# N50, maximum, total, Ns
import sys, argparse

# colored print  borrow from https://python-forum.io/Thread-Colored-text-output-in-python-3
black = lambda text: '\033[0;30m' + text + '\033[0m'
red = lambda text: '\033[0;31m' + text + '\033[0m'
green = lambda text: '\033[0;32m' + text + '\033[0m'
yellow = lambda text: '\033[0;33m' + text + '\033[0m'
blue = lambda text: '\033[0;34m' + text + '\033[0m'
magenta = lambda text: '\033[0;35m' + text + '\033[0m'
cyan = lambda text: '\033[0;36m' + text + '\033[0m'
white = lambda text: '\033[0;37m' + text + '\033[0m'

def col_evd(min_suprt, ntech, evd_fn, ctg_evd):
    tsc = 0
    tlen = 0
    prers = -1
    prere = -1
    prectg = ""
    ctge = -1
    suprt_tech_distri = [0] * (ntech + 1) # need to change if applied techs are changed
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
            suprt_tech_distri[ntech] += (re  - rs + 1)
            ctge = re
        if prers != -1:
            ctg_evd[prectg].append([prers, prere])
        if ctge != -1:
            tlen += ctge
        f.close()
                                                     
    return [tsc, tlen, suprt_tech_distri]

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
    idx = [-1] * 6 # from N50 - N100
    for z in range(lls):
        for i in range(5, 11):
            if suml[z] >= i * suma / 10:
                if idx[i-5] == -1:
                    idx[i-5] = z
    # print (ls) 
    v = [ls[e] for e in idx]
    return (suma, maxa, lls, idx, v)

def print_stat(tscore, suptd, tlen, suma, lls, maxa, idx, v, ntech):
    print (green("assembly scores:"))
    print ("absolute assembly score: {0:.2f}".format(tscore/tlen))
    print ("relative assembly score: {0:.2f}".format(tscore/tlen/ntech))
    print ("")
    print (green("support technology distribution (%) [0-{0} techs]:".format(ntech)) + " {1}".format(ntech, ' '.join(["{0:.2f}".format(x/tlen * 100) for x in suptd])))
    print ("")
    print (green("reliable blocks statistics [>=2 techs]")) 
    print ("sum: {0}, largest: {1}, average: {2:.2f}".format(suma, maxa, suma/lls)) 
    # print (v)
    for z in [50, 60, 70, 80, 90, 100]:
        print ("N{0}: {1}, L{0}: {2}".format(z, v[z//10-5], idx[z//10 - 5])) 

def worker(opts):
    min_suprt = opts.minsup 
    acc_fn = opts.acc_fn 
    ntech = opts.ntech
    
    ctg_evd = {}
    [tscore, tlen, suptd] =col_evd(min_suprt, ntech, acc_fn, ctg_evd)
    (suma, maxa, lls, idx, v) = stat(ctg_evd) 
    print_stat(tscore, suptd, tlen, suma, lls, maxa, idx, v, ntech)    

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Genome Comparison plot')
    parser.add_argument('-m', '--minsup', type=int, action="store", dest = "minsup", help ='minimum support technology', default=2)
    parser.add_argument('-n', '--ntech', type=int, action="store", dest = "ntech", help ='number of technologies', default=4)
    parser.add_argument('acc_fn', type=str, action="store", help = "evidence accumulation bed file")
    opts = parser.parse_args()
    sys.exit(worker(opts))


