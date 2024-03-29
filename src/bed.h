/*
 * =====================================================================================
 *
 *       Filename:  bed.h
 *
 *    Description:  header for bed file 
 *
 *        Version:  1.0
 *        Created:  27/09/2018 11:42:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dengfeng Guan (D. Guan), dfguan@hit.edu.cn
 *   Organization:  Center for Bioinformatics, Harbin Institute of Technology
 *
 * =====================================================================================
 */

#ifndef _BED_H
#define _BED_H

#include <stdint.h>
#include <sys/types.h>

#ifndef KSTRING_T
#define KSTRING_T kstring_t
typedef struct __kstring_t {
	size_t l, m;
	char *s;
} kstring_t;
#endif

typedef struct {
	void *fp;
	kstring_t buf;
	int is_eof;
}bed_file_t;

typedef struct {
	char *type;
	char *desc;
}bed_hdr_t;

typedef struct {
	char		*ctgn;
	uint32_t	s, e;
	uint8_t		n_tec;
	char		*tech;
}bed_rec_t;

typedef struct {
	char *ctgn;
	uint32_t s, e;
}gap_rec_t;

#ifdef __cplusplus
extern "C" {
#endif

bed_file_t *bed_open(const char *fn);
int bed_close(bed_file_t *pf);
int bed_read(bed_file_t *pf, bed_rec_t *r);
int bed_hdr_read(bed_file_t *pf, bed_hdr_t *h);
#ifdef __cplusplus
}
#endif

#endif


