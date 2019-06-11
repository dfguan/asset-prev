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

def col_gaps(gap_fn):
    gaps = {}
    with open(gap_fn) as f:
        for ln in f:
            lnlst = ln.strip().split('\t')
            chrn = lnlst[0]
            s = int(lnlst[1])
            e = int(lnlst[2])
            if chrn not in gaps:
                gaps[chrn] = []
            gaps[chrn].append([s, e])



def bin_srch(ary, ary_s, ary_e, s, e):
    if ary_e - ary_s < 0:
        return 0
    l = ary_s
    h = ary_e
    while l < h:
        m = ( l + h) >> 1
        if ary[m][0] > s:
            h = m - 1
        elif ary[m][1] < s:
            l = m + 1
        else:
            l = h = m



def col_evd(min_suprt, ntech, evd_fn, ctg_evd):
    tsc = 0
    tlen = 0
    prers = -1
    prere = -1
    prectg = ""
    ctge = -1
    gaps = 0
    suprt_tech_distri = [0] * (ntech + 1) # need to change if applied techs are changed
    gap_tech_distri = [0] * (ntech+1)
    ctgnd = {}
    with open(evd_fn) as f:
        for ln in f:
            lnlist = ln.strip().split('\t')
            ctgn = lnlist[0]
            rs = int(lnlist[1])
            re = int(lnlist[2])
            ntech = int(lnlist[3])
            isgap = 1 if lnlist[5] == 'G' else 0 
            # print (lnlist[5])
            if ctgn not in ctgnd: # contig name is in order, this is a new contig
                if prers != -1:
                    ctg_evd.append([prere - prers, gaps])
                if ctge != -1:
                    tlen += ctge

                prers = -1
                prere = -1
                prectg = ctgn
                gaps = 0
                ctgnd[ctgn] = 1
            if ntech >= min_suprt:
                if rs > prere:
                    if prers != -1:
                        ctg_evd.append([prere - prers, gaps])
                    prers = rs
                    prere = re
                    if isgap:
                        gaps += (re - rs)
                else:
                    prere = re
                    if isgap:
                        gaps += (re - rs)
            else:
                if prers != -1:
                    ctg_evd.append([prere - prers, gaps])
                prers = -1
                prere = -1
                gaps  = 0
            tsc += (re - rs + 1) * ntech
            suprt_tech_distri[ntech] += (re  - rs)
            if isgap:
                gap_tech_distri[ntech] += (re - rs)
            ctge = re
        if prers != -1:
            ctg_evd.append([prere - prers, gaps])
        if ctge != -1:
            tlen += ctge
        f.close()
                                                     
    return [tsc, tlen, suprt_tech_distri, gap_tech_distri]



def stat(ctg_evd, contain_gaps):
		# new list of value   
    ls = []
    # for k in ctg_evd:
        # for z in ctg_evd[k]:
            # print ("{0}\t{1}\t{2}\t{3}".format(k, z[0], z[1], z[1] - z[0] + 1))
            # ls.append(z[1] - z[0] + 1)
    if contain_gaps:
        for k in ctg_evd:
            if k[0]:
                ls.append(k[0])
    else:
        for k in ctg_evd:
            if k[0] - k[1]:
                ls.append(k[0] -k[1])

    maxa = max(ls) 
    suma = sum(ls)
    # print (suma)
    lls = len(ls)
    ls.sort(reverse=True)
    # ctg_evd.sort(key = lambda x:x[0], reverse = True)
    # print (ls)
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

def print_stat(tscore, suptd, gaptd, tlen, stat_gaps, stat_nogaps, min_suprt, ntech):
    print (green("assembly scores:"))
    print ("absolute assembly score: {0:.2f}".format(tscore/tlen))
    print ("relative assembly score: {0:.2f}".format(tscore/tlen/ntech))
    print ("")
    min_suprt_frac = 0
    min_suprt_agct_frac = 0
    for i in range(min_suprt, ntech+1):
        min_suprt_frac += suptd[i]
        min_suprt_agct_frac += (suptd[i] - gaptd[i])
    min_suprt_agct_frac /= min_suprt_frac
    min_suprt_frac /= tlen
    print (green("support technology distribution (%, %-Non-Ns) [0-{0} techs]:".format(ntech)) + " {1} {2:.2f} {3:.2f}".format(ntech, ' '.join(["{0:.2f} {1:.2f}".format(suptd[i]/tlen * 100, (suptd[i] - gaptd[i])/suptd[i] * 100) for i in range(ntech + 1)]), min_suprt_frac * 100, min_suprt_agct_frac * 100))
    print ("")
    header = "reliable blocks statistics [>=2 techs]: "
    for suma, maxa, lls, idx, v in [stat_gaps, stat_nogaps]:
        print (green(header)) 
        print ("sum: {0}, largest: {1}, average: {2:.2f}".format(suma, maxa, suma/lls)) 
        # print (v)
        for z in [50, 60, 70, 80, 90, 100]:
            print ("N{0}: {1}, L{0}: {2}".format(z, v[z//10-5], idx[z//10 - 5])) 
        header = "reliable blocks statistics [>=2 techs] (Non-Ns): "
def worker(opts):
    min_suprt = opts.minsup 
    acc_fn = opts.acc_fn 
    ntech = opts.ntech
    
    ctg_evd = [] 
    [tscore, tlen, suptd, gaptd] =col_evd(min_suprt, ntech, acc_fn, ctg_evd)
    stat_gaps = stat(ctg_evd, 1) 
    stat_no_gaps = stat(ctg_evd, 0) 
    # print (gaptd)
    print_stat(tscore, suptd, gaptd, tlen, stat_gaps, stat_no_gaps, min_suprt, ntech)    

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Genome Comparison plot')
    parser.add_argument('-m', '--minsup', type=int, action="store", dest = "minsup", help ='minimum support technology', default=2)
    parser.add_argument('-n', '--ntech', type=int, action="store", dest = "ntech", help ='number of technologies', default=4)
    parser.add_argument('acc_fn', type=str, action="store", help = "evidence accumulation bed file")
    opts = parser.parse_args()
    sys.exit(worker(opts))


