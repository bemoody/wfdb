/* file: ahaecg2mit.c		G. Moody		7 May 2008
.				Last revised:	       24 August 2010
Convert a *.ecg file from an AHA Database DVD to WFDB-compatible format
*/

#include <stdio.h>
#include <wfdb/wfdb.h>
#include <wfdb/ecgmap.h>

main(int argc, char **argv)
{
    char *p, *record;
    int i, sflag = 0;
    FILE *ifile;
    void process(char *r, FILE *f);

    if (argc < 2 || strcmp(argv[1], "-h") == 0) {
	fprintf(stderr, "usage: %s [-s] RECORD.ecg [RECORD.ecg]...\n", argv[0]);
	fprintf(stderr, " (use -s to make short-format 35-minute records)\n");
	exit(1);
    }
    if (strcmp(argv[i = 1], "-s") == 0) {
	sflag = 1;	/* produce short-format (35-minute) records */
	i = 2;
    }
    for ( ; i < argc; i++) {
	p = argv[i] + strlen(argv[i]) - 4;	/* pointer to '.ecg' */
	if (strcmp(".ecg", p) && strcmp(".ECG", p)) {
	    fprintf(stderr, "%s: ignoring '%s'\n", argv[0], argv[i]);
	    continue;	/* not an .ecg or .ECG file */
	}
	if ((ifile = fopen(argv[i], "rb")) == NULL) {
	    fprintf(stderr, "%s: can't open '%s'\n", argv[0], argv[1]);
	    continue;
	}
	*p = '\0';	/* strip off extension */
	record = p - 4;	/* AHA record names are 4 (ASCII) digits */
	if (sflag) {/* skip first 145 minutes if making a short-format record */
	    fseek(ifile, 145L*60L*250L*5L, SEEK_SET);
	    record[1] += 2;	/* fix record name (n0nn->n2nn, n1nn->n3nn) */
	}
	process(record, ifile);
	fclose(ifile);
    }
    exit(0);
}

void process(char *record, FILE *ifile)
{
    char ofname[10], data[5];
    WFDB_Time t = -1;
    static WFDB_Siginfo si[2];
    static WFDB_Anninfo ai;
    static WFDB_Sample v[2];
    static WFDB_Annotation a;

    setsampfreq(250.0);	/* AHA DB is sampled at 250 Hz for each signal */
    sprintf(ofname, "%s.dat", record);
    si[0].fname = ofname;
    si[0].desc = "ECG0";
    si[0].units = "mV";
    si[0].gain = 400;
    si[0].fmt = 212;
    si[0].adcres = 12;
    si[1] = si[0];
    si[1].desc = "ECG1";
    ai.name = "atr";
    ai.stat = WFDB_WRITE;
    if (osigfopen(si, 2) != 2 || annopen(record, &ai, 1) < 0) {
	wfdbquit();
	return;
    }
    while (fread(data, 1, 5, ifile) == 5) {
	v[0] = (data[0] & 0xff) | ((data[1] << 8) & 0xff00);
	v[1] = (data[2] & 0xff) | ((data[3] << 8) & 0xff00);
	(void)putvec(v);
	if (data[4] != '.') {
	    a.anntyp = ammap(data[4]);
	    a.subtyp = (data[4] == 'U' ? -1 : 0);
	    a.time = t;
	    (void)putann(0, &a);
	}
	t++;
    }
    (void)newheader(record);
    wfdbquit();
    fprintf(stderr, "wrote %s.atr, %s.dat, and %s.hea\n", record,record,record);
    return;
}
