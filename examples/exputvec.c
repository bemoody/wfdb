#include <wfdb/wfdb.h>

main()
{
    int i, nsig, v[WFDB_MAXSIG];
    static WFDB_Siginfo s[WFDB_MAXSIG];

    if ((nsig = isigopen("100s", s, WFDB_MAXSIG)) < 1 ||
        osigopen("8l", s, nsig) < 1)
        exit(1);
    for (i = 0; i < 10; i++)
        if (getvec(v) < 0 || putvec(v) < 0)
            break;
    wfdbquit();
    exit(0);
}
