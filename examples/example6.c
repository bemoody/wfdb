#include <stdio.h>
#include <wfdb/wfdb.h>

main(argc, argv)
int argc;
char *argv[];
{
    static WFDB_Siginfo s[WFDB_MAXSIG];
    static int i, nsig, nsamp=1000, vin[WFDB_MAXSIG], vout[WFDB_MAXSIG];

    if (argc < 2) {
        fprintf(stderr, "usage: %s record\n", argv[0]); exit(1);
    }
    if ((nsig = isigopen(argv[1], s, WFDB_MAXSIG)) <= 0) exit(2);
    if (osigopen("8l", s, nsig) <= 0) exit(3);
    while (nsamp-- > 0 && getvec(vin) > 0) {
        for (i = 0; i < nsig; i++)
            vout[i] -= vin[i];
        if (putvec(vout) < 0) break;
        for (i = 0; i < nsig; i++)
            vout[i] = vin[i];
    }
    (void)newheader("dif");
    wfdbquit();
    exit(0);
}
