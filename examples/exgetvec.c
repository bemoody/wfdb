#include <wfdb/wfdb.h>

main()
{
    int i, j, nsig, v[WFDB_MAXSIG];
    static WFDB_Siginfo s[WFDB_MAXSIG];

    nsig = isigopen("100s", s, WFDB_MAXSIG);
    if (nsig < 1)
        exit(1);
    for (i = 0; i < 10; i++) {
        if (getvec(v) < 0)
            break;
        for (j = 0; j < nsig; j++)
            printf("%8d", v[j]);
        printf("\n");
    }
    exit(0);
}
