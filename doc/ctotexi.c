#include <stdio.h>

main()
{
  char ibuf[120], *p;
  int col, line = 0;

  while (fgets(ibuf, sizeof(ibuf), stdin)) {
    if (++line < 10) printf(" ");
    printf("@i{%d}  ", line);
    for (col = 0, p = ibuf; *p; p++, col++) {
      if (*p == '{' || *p == '}' || *p == '@')
	putchar('@');
      else if (*p == '\t') {
	*p = ' ';
	while (++col % 8)
	  putchar(' ');
      }
      putchar(*p);
    }
  }
}
