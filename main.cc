/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "Bruinbase.h"
// #include "SqlEngine.h"
#include "BTreeNode.h"
#include <cstdio>
 #include <stdio.h>
#include <string.h>
#include <stdlib.h>

void dump_buffer(char *buffer);

int main()
{
  // run the SQL engine taking user commands from standard input (console).
  // SqlEngine::run(stdin);
	BTLeafNode n1;
	memset(n1.buffer, 0, sizeof(n1.buffer));

	leafCursor l1;
	l1.key = 1;
	l1.rid.pid = 2;
	l1.rid.sid = 1;
	leafCursor *p = (leafCursor *) n1.buffer;
	*p = l1;
	p++;
	l1.key = 2;
	l1.rid.pid = 2;
	l1.rid.sid = 2;
	*p = l1;
	p++;
	n1.setNextNodePtr(4);

	dump_buffer(n1.buffer);
	l1.key = 3;
	l1.rid.pid = 4;
	l1.rid.sid = 2;

	n1.insert(l1.key,l1.rid);
	dump_buffer(n1.buffer);

  return 0;
}

void dump_buffer(char *buffer)
{
	printf("buffer is:\n");
	for (int i = 0; i < 1024; ++i)
	{
		printf("%d", buffer[i]);
	}
	printf("\n");
}