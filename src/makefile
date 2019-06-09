CC      =  gcc
CFLAGS  =  -g -Wall -D VERBOSE -O2  
LDFLAGS = -lz

#OBJS = gfa.o opt.o paf.o sdict.o eg.o 
PROG = ast_pb ast_10x ast_hic ast_bion acc union pchlst detgaps acc2 # ast_postproc

.SUFFIXS:.c .o

all:$(PROG)
	mkdir -p ../bin && cp $(PROG) ../bin

ast_pb: ast_pb.o ast.o sdict.o paf.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

ast_10x: ast_10x.o ast.o sdict.o bamlite.o bed.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

ast_hic: ast_hic.o ast.o sdict.o bamlite.o  bed.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

ast_bion: ast_bion.o amap.o sdict.o ast.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

detgaps: det_gaps.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

acc: acc.o sdict.o bed.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

acc2: acc2.o sdict.o bed.o ast.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

join:joinh.o sdict.o bamlite.o 
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

union:union_beds.o sdict.o bed.o ast.o 
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

pchlst: gen_pchlst.o bed.o ast.o sdict.o bed.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
ast_postproc: ast_postproc.o bed.o ast.o paf.o sdict.o 
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.c .o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(PROG)

	


