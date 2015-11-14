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
	n1.setNextNodePtr(4);

	leafCursor l1;
	l1.rid.pid = 0;
	l1.rid.sid = 0;

	for (int i = 0; i < 47; i++)
	{
		l1.key = i+1;
		n1.insert(l1.key,l1.rid);
		dump_buffer(n1.buffer);
	}
	for (int i = 48; i < 86; i = i + 1)
	{
		l1.key = i+1;
		n1.insert(l1.key,l1.rid);
		dump_buffer(n1.buffer);
	}

	BTLeafNode n2;
	int siblingKey;
	n1.insertAndSplit( 48, l1.rid, n2,siblingKey);
	printf("n1 is:\n");
	dump_buffer(n1.buffer);
	printf("n2 is:\n");
	dump_buffer(n2.buffer);
	printf("siblingKey is:\n");
	printf("%d\n", siblingKey);

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