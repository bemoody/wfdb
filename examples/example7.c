#include <stdio.h>
#include <wfdb/wfdb.h>
#define BUFLN 512
int nsig, sample_ok = 1;
WFDB_Sample *sbuf;

sample(s, t)
int s;
long t;
{
    static long tt = -1L;

    if (t <= tt - BUFLN)
        fprintf(stderr, "sample: buffer too short\n");
    while (t > tt)
        if (getvec(sbuf + nsig * ((++tt)&(BUFLN-1))) < 0)
	    sample_ok = 0;
    return (*(sbuf + nsig * (t&(BUFLN-1)) + s));
}

main(argc, argv)
int argc;
char *argv[];
{
    double *c, one = 1.0, vv, atof();
    int i, j, nc = argc - 4;
    long nsamp, t;
    static WFDB_Sample *v;
    static WFDB_Siginfo *s;

    if (argc < 4) {
        fprintf(stderr,
          "usage: %s record start duration [ coefficients ... ]\n",
                argv[0]);
        exit(1);
    }
    if (nc < 1) {
        nc = 1; c = &one;
    }
    else if (nc >= BUFLN ||
             (c = (double *)calloc(nc, sizeof(double))) == NULL) {
        fprintf(stderr, "%s: too many coefficients\n", argv[0]);
        exit(2);
    }
    for (i = 0; i < nc; i++)
        c[i] = atof(argv[i+4]);
    if ((nsig = isigopen(argv[1], NULL, 0)) < 1)
        exit(3);
    s = (WFDB_Siginfo *)malloc(nsig * sizeof(WFDB_Siginfo));
    v = (WFDB_Sample *)malloc(nsig * sizeof(WFDB_Sample));
    sbuf = (WFDB_Sample *)malloc(nsig * sizeof(WFDB_Sample) * BUFLN);
    if (s == NULL || v == NULL || sbuf == NULL) {
	fprintf(stderr, "insufficient memory\n");
	exit(3);
    }
    if (isigopen(argv[1], s, nsig) != nsig)
        exit(3);
    if (isigsettime(strtim(argv[2])) < 0)
        exit(4);
    if ((nsamp = strtim(argv[3])) < 1) {
        fprintf(stderr, "%s: inappropriate value for duration\n",
                argv[0]);
        exit(5);
    }
    if (osigopen("16l", s, nsig) != nsig)
        exit(6);

    for (t = 0; t < nsamp && sample_ok; t++) {
        for (j = 0; j < nsig; j++) {
            for (i = 0, vv = 0.; i < nc; i++)
                if (c[i] != 0.) vv += c[i]*sample(j, t+i);
            v[j] = (int)vv;
        }
        if (putvec(v) < 0) break;
    }

    (void)newheader("out");
    wfdbquit();
    exit(0);
}
