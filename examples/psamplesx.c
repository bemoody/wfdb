#include <wfdb/wfdb.h>

main(int argc, char **argv)
{
    int i, v[2];
    WFDB_Siginfo s[2];
    WFDB_Frequency f;
    WFDB_Time t;

    if (argc > 1) sscanf(argv[1], "%lf", &f);
    else f = sampfreq("100s");

    if (isigopen("100s", s, 2) < 1)
	exit(1);
    setifreq(f);
    t = strtim("1");
    isigsettime(t);
    for (i = 0; i < (int)(f + 0.5); i++) {
	if (getvec(v) < 0)
	    break;
	printf("%d\t%d\n", v[0], v[1]);
    }
    for (i = 0; i < (int)(f/10); i++)
	printf("1000\t1000\n");
    t = strtim("3");
    isigsettime(t);
    for (i = 0; i < (int)(f + 0.5); i++) {
	if (getvec(v) < 0)
	    break;
	printf("%d\t%d\n", v[0], v[1]);
    }
    isigsettime((WFDB_Time)f);
    for (i = 0; i < (int)(f/10); i++)
	printf("1000\t1000\n");
    t = strtim("1");
    isigsettime(t);
    for (i = 0; i < (int)(f + 0.5); i++) {
	if (getvec(v) < 0)
	    break;
	printf("%d\t%d\n", v[0], v[1]);
    }
    exit(0);
}
