import sys

black = lambda text: '\033[0;30m' + text + '\033[0m'
red = lambda text: '\033[0;31m' + text + '\033[0m'
green = lambda text: '\033[0;32m' + text + '\033[0m'
yellow = lambda text: '\033[0;33m' + text + '\033[0m'
blue = lambda text: '\033[0;34m' + text + '\033[0m'
magenta = lambda text: '\033[0;35m' + text + '\033[0m'
cyan = lambda text: '\033[0;36m' + text + '\033[0m'
white = lambda text: '\033[0;37m' + text + '\033[0m'
def stat_single_bed(bed_fn):
    lst = []
    with open(bed_fn) as f:
        for ln in f:
            if ln[0:5] == "track":
                continue
            lnlst = ln.strip().split('\t')
            rs = int(lnlst[1])
            re = int(lnlst[2])
            lst.append(re - rs)
        f.close()

    maxa = max(lst) 
    suma = sum(lst)
    # print (suma)
    lls = len(lst)
    lst.sort(reverse=True)
    # ctg_evd.sort(key = lambda x:x[0], reverse = True)
    # print (ls)
    suml = [sum(lst[0:z + 1]) for z in range(lls)] 
    idx = [-1] * 6 # from N50 - N100
    for z in range(lls):
        for i in range(5, 11):
            if suml[z] >= i * suma / 10:
                if idx[i-5] == -1:
                    idx[i-5] = z
    # print (ls) 
    v = [lst[e] for e in idx]
    print (green("stat for " + bed_fn))
    print ("sum: {0}, largest: {1}, average: {2:.2f}".format(suma, maxa, suma/lls)) 
    # print (v)
    for z in [50, 60, 70, 80, 90, 100]:
        print ("N{0}: {1}, L{0}: {2}".format(z, v[z//10-5], idx[z//10 - 5] + 1)) 
    print ("google sheet input: {0} {1} {2} {3} {4}".format(lls, suma, maxa, int(suma/lls), v[0]))

if __name__ == "__main__":
    stat_single_bed(sys.argv[1])
