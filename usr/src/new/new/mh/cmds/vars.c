	register char *cp;
	register int i,j;
	register FILE *in;
	char name[NAMESZ], field[BUFSIZ];
	char *drft, *msgid, *replto, *from, *cc, *sub, *date, *to;
	int state, out, status, intr;
	int pid, wpid;
	char **argp, *address;
	char ackfile[10];
	char buf[BUFSIZ];
