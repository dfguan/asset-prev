/*
 * =====================================================================================
 *
 *       Filename:  aa_10x.c
 *
 *    Description:  assembly assessment with 10x 
 *
 *        Version:  1.0
 *        Created:  15/09/2018 15:42:59
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dengfeng Guan (D. Guan), dfguan@hit.edu.cn
 *   Organization:  Center for Bioinformatics, Harbin Institute of Technology
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>


#include "ast.h"
#include "bamlite.h"
#include "sdict.h"
#include "kvec.h"
#include "ksort.h"
#include "bed.h"

#define BC_LEN 16

typedef struct {
	int mq:15, rev:1, as:16;
	uint32_t s, e, tid;
}aln_inf_t;

typedef struct {
	/*int mq:15, rev:1, as:16;*/
	uint32_t tid:31, rev:1;
	uint32_t ntid:31, nrev:1;
	uint32_t s, e;
}aln2_inf_t;




typedef struct {
	uint32_t s, e; //no more than 
	uint64_t bctn;
}bc_t;


#define bc_key(a) ((a).bctn)
KRADIX_SORT_INIT(bct, bc_t, bc_key, 8)

#define cord_key(a) ((a).s)
KRADIX_SORT_INIT(cord, cors, cord_key, 4);
	
typedef struct {
	uint32_t n, m;
	bc_t *ary;
}bc_ary_t;

void bc_ary_push(bc_ary_t *l, bc_t *z)
{
	uint32_t max = -1;
	if (l->n >= l->m) {
		if (l->m > (max >> 1)) {
			fprintf(stderr, "Too many values here\n");
			exit(1);
		} else 
			l->m = l->m ? l->m << 1 : 16;
		l->ary = realloc(l->ary, sizeof(bc_t) * l->m);//beware of overflow
	}
	l->ary[l->n++] = *z;
}

uint32_t gap_sz(uint32_t s, uint32_t e, cord_t *ct) //search first greater
{
	cors *c = ct->coords;
	uint32_t nc = ct->n;
	uint32_t gap_size = 0;
	if (!nc) return gap_size; 
	if (s < c[0].s || s > c[nc-1].e) return gap_size;
	uint32_t mid;
	uint32_t h = nc, l = 0;
	while (l != h) {
		mid = (l+h)>>1;
		if (c[mid].s <= s) l = mid + 1;
		else 
			h = mid;
	}	
	for (;l < nc && c[l].e <= e; ++l) gap_size += (c[l].e - c[l].s + 1);  
	return gap_size;
}

void col_bcnt(aln_inf_t  *fal, uint32_t bc, int min_mq, uint32_t max_is, bc_ary_t *l)
{
	if (fal->mq > min_mq) {
		uint32_t s = fal->s;
		uint32_t e = fal->e;
		/*s = s << 1;*/
		/*e = e << 1 | 1; */
		/*if (e - s < max_ins_len) {*/
			/*if (opt) {*/
				/*uint32_t tmp = s;*/
				/*s = e;*/
				/*e = tmp;*/
			/*}*/
		if (e - s < max_is) {
			bc_t t = (bc_t) {s, e, (uint64_t)bc << 32 | fal->tid};
			
			bc_ary_push(l, &t);	
		}
		/*}*/
		/*pos_push(&d->ctg_pos[fal[0].tid], s);*/
		/*pos_push(&d->ctg_pos[fal[0].tid], e); // we init */
		/*ctg_pos_push(&d[fal[0].tid], s);k*/
		/*ctg_pos_push(&d[fal[0].tid], e);*/
	}
}

uint32_t get_target_end(uint32_t *cigar, int n_cigar, uint32_t s)
{
	int i = 0;
	for ( i = 0; i < n_cigar; ++i) {
		uint8_t c  = bam_cigar_opchr(cigar[i]);
		if (c == 'M' || c == 'D') 
			s += cigar[i] >> BAM_CIGAR_SHIFT;
	}	
	return s;
}


/*bc_ary_t *proc_bam(char *srt_bam_fn, int min_as, int min_mq, uint32_t max_ins_len, sdict_t *ctgs, int opt)*/
int proc_bam(char *bam_fn, int min_mq, uint32_t max_is, sdict_t *ctgs, int opt, bc_ary_t *bc_l)
{
	bamFile fp;
	bam_header_t *h;
	bam1_t *b;
	fp = bam_open(bam_fn, "r"); //should check if bam is sorted
	if (fp == 0) {
		fprintf(stderr, "[E::%s] fail to open %s\n", __func__, bam_fn);
		return -1;
	}
	
	h = bam_header_read(fp);
	b = bam_init1();
	
	/*ctg_pos_t *d = ctg_pos_init();*/
	int i;
	for ( i = 0; i < h->n_targets; ++i) 
		sd_put(ctgs, h->target_name[i], h->target_len[i]);
		/*ctg_pos_push(d, i);*/

	char *cur_qn = NULL, *cur_bc = NULL;
	/*int32_t cur_l = 0;*/
	sdict_t* bc_n = sd_init();
	long bam_cnt = 0;
	int is_set = 0;
	aln_inf_t aln;
	int aln_cnt = 0;
	uint8_t rev;
	while (1) {
		//segment were mapped 
		if (bam_read1(fp, b) >= 0 ) {
			if (!cur_qn || strcmp(cur_qn, bam1_qname(b)) != 0) {
				/*fprintf(stderr, "%d\t%d\t%d\n", aln_cnt, rev, aln.mq);*/
				if (aln_cnt == 2 && (rev == 1 || rev == 2)) col_bcnt(&aln, sd_put(bc_n, cur_bc, BC_LEN), min_mq, max_is, bc_l);
				aln_cnt = 0;	
				rev = 0;
				is_set = 0;
				if (cur_qn) free(cur_qn); 
				cur_qn = strdup(bam1_qname(b));
				cur_bc = cur_qn + b->core.l_qname - BC_LEN;
			}
			if (b->core.flag & 0x4) continue; //not aligned
			++aln_cnt;
			rev = (rev << 1) | !!(b->core.flag & 0x10);
			if (b->core.isize > 0 && !is_set) {
				uint8_t *s = bam_aux_get(b, "AS");
				if (s) aln.as = *(int32_t *)(s+1); else aln.as = -1;	
				aln.s = opt ? get_target_end(bam1_cigar(b), b->core.n_cigar, b->core.pos) :  b->core.pos + 1; // one-based 
				aln.mq = b->core.qual;
				aln.tid = b->core.tid;
				/*uint32_t e = get_target_end(bam1_cigar(b), b->core.n_cigar, aln.s);			*/
				aln.e = aln.s + b->core.isize - 1; //fully-closed 	
			} 
			if (opt && b->core.isize < 0) 
				aln.e = b->core.pos + 1;
			
			
			if ((++bam_cnt % 1000000) == 0) fprintf(stderr, "[M::%s] processing %ld bams\n", __func__, bam_cnt); 
		} else {
			/*fprintf(stderr, "%d\t%d\t%d\n", aln_cnt, rev, aln.mq);*/
			if (aln_cnt == 2 && (rev == 1 || rev == 2)) col_bcnt(&aln, sd_put(bc_n, cur_bc, BC_LEN), min_mq, max_is, bc_l);
			break;	
		}
	}
	fprintf(stderr, "[M::%s] finish processing %ld bams\n", __func__, bam_cnt); 
	
	bam_destroy1(b);
	bam_header_destroy(h);
	bam_close(fp);
	sd_destroy(bc_n);	
	return 0;
}

void insert_sort(bc_t *b, bc_t *e)
{
	bc_t *i, *j, swap_tmp;
		for (i = b + 1; i < e; ++i)										
			for (j = i; j > b && ((j-1)->s > j->s || ((j-1)->s == j->s && (j-1)->e > j->e)); --j) {			
				swap_tmp = *j; *j = *(j-1); *(j-1) = swap_tmp;			
			}															
}
void srt_by_nm_loc(bc_t *s, bc_t *e)
{
	bc_t *i = s;
	while (i < e) {
		bc_t *j;
		for (j = i + 1; j < e && j->bctn == i->bctn; ++j);
		insert_sort(i, j);
		i = j;
	}	
}

ctg_pos_t *col_pos(cord_t *cc, int n, sdict_t *ctgs)
{
	ctg_pos_t *d = ctg_pos_init();
	int k;
	for ( k = 0;k < n; ++k) {
		ctg_pos_push(d, k);
		cors *e = cc[k].coords;
		size_t n_e = cc[k].n;
		radix_sort_cord(e, e + n_e);
		size_t i = 0, j;
		uint32_t st,ed;
		
		while (i < n_e) {
			st = e[i].s, ed= e[i].e;
					/*fprintf(stderr, "h%s\t%u\t%u\n",ctgs->seq[k].name, st, ed);*/
			for ( j = i + 1; j <= n_e; ++j) {
					/*fprintf(stderr, "t%s\t%u\t%u\n",ctgs->seq[k].name, st, ed);*/
				if (j == n_e || e[j].s != e[i].s) {
					/*fprintf(stderr, "d%s\t%u\t%u\n",ctgs->seq[k].name, st, ed);*/
					pos_push(&d->ctg_pos[k], st << 1);
					pos_push(&d->ctg_pos[k], ed << 1 | 1);
					i = j;
					break;
				} else {
					ed = max(ed, e[j].e);
				}
			} 
		}	
	} 
	return d;	
}

void init_gaps(char *gap_fn, ns_t *ns, sdict_t *ctgs, uint32_t min_len)
{
	bed_file_t* bf = bed_open(gap_fn);
	bed_rec_t r;
	ns->ct = calloc(ctgs->n_seq, sizeof(cord_t));
	while (bed_read(bf, &r) >= 0) {
		uint32_t ind = sd_get(ctgs, r.ctgn);
		if (r.e - r.s > min_len) {
			cors tmp = (cors){r.s + 1, r.e}; //to 1 based
			cord_push(&ns->ct[ind], &tmp);				
		}
	}
	bed_close(bf);
}
/*ctg_pos_t *col_pos(bc_ary_t *bc_l, uint32_t min_bc, uint32_t max_bc, uint32_t min_inner_bcn, uint32_t min_mol_len, int n_targets)*/
	/*cord_t *cc = col_cords(bc_l, min_bc, max_bc, min_inner_bcn, max_span, min_mol_len, ctgs->n_seq, ctgs);	*/
cord_t *col_cords(bc_ary_t *bc_l, uint32_t min_bc, uint32_t max_bc, uint32_t min_inner_bcn, uint32_t max_span, uint32_t min_mol_len, int n_targets, sdict_t *ctgs, char *gap_fn)
{

	int k;

	ns_t *ns = calloc(1, sizeof(ns_t));
	init_gaps(gap_fn, ns, ctgs, 0);	

	cord_t *cc = calloc(n_targets, sizeof(cord_t));
	
	radix_sort_bct(bc_l->ary, bc_l->ary + bc_l->n);	
	uint32_t n = bc_l->n;
	bc_t *p = bc_l->ary;

	uint32_t i = 0;
	while (i < n) {
		uint32_t z;
		for ( z = i; z < n && (p[z].bctn >> 32) == (p[i].bctn >> 32); ++z);
		uint32_t n_bc = z - i;
		if (n_bc > min_bc && n_bc < max_bc) {
			srt_by_nm_loc(p+i, p+z); //sort by ctg name and locus
			uint32_t s = p[i].s,e = p[i].e;
			uint32_t j = i + 1;
			
			/*fprintf(stderr, "%u\t%u\n",j,z);*/
			while (j <= z) {
				if (j == z || p[j].bctn != p[i].bctn || (p[j].s - p[j-1].s > max_span && p[j].s - p[j-1].s - gap_sz(p[j-1].s, p[j].s, &ns->ct[p[i].bctn & 0xFFFFFFFF]) > max_span)) {
					/*if (s > e) fprintf(stderr, "woofy%u\t%u\n", s, e);*/
					/*fprintf(stderr, "%s\t%u\t%u\t%u\n",ctgs->seq[p[i].bctn&0xFFFFFFFF].name, s, e, j - i);*/
					if (j - i > min_inner_bcn && e - s > min_mol_len) {
						cors tmp = (cors) {s, e}; 
						cord_push(&cc[p[i].bctn & 0xFFFFFFFF], &tmp);
						/*fprintf(stderr, "%s\t%u\t%u\n",ctgs->seq[p[i].bctn&0xFFFFFFFF].name, s, e);*/
					} 					
					/*pos_push(&d->ctg_pos[p[i].bctn & 0xFFFFFFFF], s << 1);*/
					/*pos_push(&d->ctg_pos[p[i].bctn & 0xFFFFFFFF], e << 1 | 1);*/
					if (j == z) break; //otherwise infinate loop
					i = j;
					s = p[i].s;
					e = p[i].e;
					j = i + 1;
				} else {
					e = max(e, p[j].e);
					/*s = min(s, p[j].s);*/
					++j;
				}
				/*if (j == z || p[j].bctn != p[i].bctn) {*/
					/*if (p[j].s - s < max_span && j - i > min_inner_bcn) {*/
						/*fprintf(stderr, "%u\t%u\n",s,e);*/
						/*pos_push(&d->ctg_pos[p[i].bctn & 0xFFFFFFFF], s << 1);*/
						/*pos_push(&d->ctg_pos[p[i].bctn & 0xFFFFFFFF], e << 1 | 1);*/
					/*}*/
					/*if (j == z) break; //otherwise infinate loop*/
					/*e = 0;*/
					/*s = -1;*/
					/*i = j;*/
				/*} else {*/
					/*e = max(e, p[j].e);*/
					/*s = min(s, p[j].s);*/
					/*++j;*/
				/*}*/
			}
		}	
		i = z;	
	}

	/*for (k=0; k < bc_l->n; ++k) {*/
		/*if (bc_l->ary[k].s > bc_l->ary[k].e)*/
			/*fprintf(stderr, "hhh%llx\t%llx\t%u\t%u\n", bc_l->ary[k].bctn >> 32, bc_l->ary[k].bctn & 0xFFFFFFFF, bc_l->ary[k].s, bc_l->ary[k].e);*/
	/*}*/
	ns_destroy(ns);
	return cc;
}

/*int aa_10x(char *srt_bam_fn, int min_as, int min_mq, int min_cov, float min_cov_rat, int max_cov, float max_cov_rat)*/
int aa_10x(char *bam_fn[], int n_bam, char *gap_fn, int min_mq, int min_cov, float min_cov_rat, uint32_t max_span, int max_cov, uint32_t max_is, int min_bc, int max_bc, uint32_t min_inner_bcn, uint32_t min_mol_len, int opt, char *out_dir)
{
	sdict_t *ctgs = sd_init();

#ifdef VERBOSE
	fprintf(stderr, "[M::%s] processing bam file\n", __func__);
#endif
	int i;
	bc_ary_t *bc_l = calloc(1, sizeof(bc_ary_t));
	for ( i = 0; i < n_bam; ++i) {
		if (proc_bam(bam_fn[i], min_mq, max_is, ctgs, opt, bc_l)) {
			return -1;	
		}	
	}	

	if (!(bc_l&&bc_l->n)) {
		fprintf(stderr, "[W::%s] none useful information, quit\n", __func__);
		return -1;
	}
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] collecting coordinates\n", __func__);
#endif
	cord_t *cc = col_cords(bc_l, min_bc, max_bc, min_inner_bcn, max_span, min_mol_len, ctgs->n_seq, ctgs, gap_fn);	
	free(bc_l->ary); free(bc_l);	
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] collecting positions\n", __func__);
#endif
	ctg_pos_t *d = col_pos(cc, ctgs->n_seq, ctgs);	
	cord_destroy(cc, ctgs->n_seq);	
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] calculating coverage\n", __func__);
#endif
	cov_ary_t *ca = cal_cov(d, ctgs);

	ctg_pos_destroy(d);
	if (!ca) {
		fprintf(stderr, "[W::%s] low quality alignments\n", __func__);
		return 0;	
	}
		
	/*csel_sup_reg(ca, min_cov_rat, min_cov, max_cov_rat, max_cov, ctgs);*/
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] selecting supported regions\n", __func__);
#endif
	char *type = "TX";
	char *desc = "10x data";

#ifdef PRINT_COVERAGE
	print_coverage_wig(ca, ctgs, type, 1024, out_dir);
	print_coverage_stat(ca, ctgs, type, out_dir);
	print_base_coverage(ca, ctgs, type, out_dir);
#endif	
	sel_sup_reg_dyn(ca,  min_cov_rat, min_cov, max_cov, ctgs, type, desc);
		
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] releasing regions\n", __func__);
#endif
	cov_ary_destroy(ca, ctgs->n_seq); //a little bit messy
	sd_destroy(ctgs);

	fprintf(stderr, "Program finished successfully\n");
	return 0;

}



int main(int argc, char *argv[])
{
	int c;
	int max_cov = 1000000, min_cov = 10, min_mq = 30;
	int min_as = 0;
	uint32_t max_span = 50000, min_inner_bcn = 3, min_mol_len = 1000;
	uint32_t min_bc = 5, max_bc = 1000000, max_is=1000;
	float min_cov_rat = .15;
	char *r;
	char *out_dir = ".";
	int option = 0; //the way to calculate molecule length //internal parameters not allowed to adjust by users
	while (~(c=getopt(argc, argv, "b:B:c:C:r:q:S:a:L:l:O:h"))) {
		switch (c) {
			case 'b': 
				min_bc = strtol(optarg, &r, 10);
				break;
			case 'B':
				max_bc = strtol(optarg, &r, 10);
				break;
			case 'c':
				min_cov = atoi(optarg); 
				break;
			case 'r':
				min_cov_rat = atof(optarg); 
				break;
			case 'C':
				max_cov = atoi(optarg); 
				break;
			case 'q':
				min_mq = atoi(optarg);
				break;
			case 'L':
				max_is = strtol(optarg, &r, 10);
				break;
			case 'S':
				max_span = strtol(optarg, &r, 10);
				break;
			case 'a':
				min_inner_bcn = strtol(optarg, &r, 10);
				break;
			case 'l':
				min_mol_len = strtol(optarg, &r, 10);
				break;
			case 'O':
				out_dir = optarg;
				break;
			default:
				if (c != 'h') fprintf(stderr, "[E::%s] undefined option %c\n", __func__, c);
help:	
				fprintf(stderr, "\nUsage: aa_10x [options] <GAP_BED> <BAM_FILEs> ...\n");
				fprintf(stderr, "Options:\n");
				fprintf(stderr, "         -b    INT      minimum barcode number for each molecule [5]\n");	
				fprintf(stderr, "         -B    INT      maximum barcode number for each molecule [inf]\n");
				fprintf(stderr, "         -c    INT      minimum coverage [10]\n");
				fprintf(stderr, "         -r    FLOAT    minimum coverage ratio [.15]\n");
				fprintf(stderr, "         -C    INT      maximum coverage [inf]\n");
				fprintf(stderr, "         -q    INT      minimum mapping quality [30]\n");
				/*fprintf(stderr, "         -S    INT      minimum aislignment score [0]\n");*/
				fprintf(stderr, "         -l    INT      minimum molecule length [1000]\n");
				fprintf(stderr, "         -S    INT      maximum spanning length [50000]\n");
				fprintf(stderr, "         -L    INT      maximum insertion length [1000]\n");
				fprintf(stderr, "         -a    INT      minimum barcode for contig [3]\n");
				fprintf(stderr, "         -h             help\n");
				return 1;	
		}		
	}
	if (optind + 1 > argc) {
		fprintf(stderr,"[E::%s] require at least one bed and bam file!\n", __func__); goto help;
	}
	
	char *gap_fn = argv[optind++];

	char **bam_fn = argv+optind;
	int n_bam = argc - optind;
	fprintf(stderr, "Program starts\n");	
	aa_10x(bam_fn, n_bam, gap_fn, min_mq, min_cov, min_cov_rat, max_span, max_cov, max_is, min_bc, max_bc, min_inner_bcn,  min_mol_len, option, out_dir);
	return 0;	
}



