/*
 * cygpath.c
 *
 *  Created on: Jun 9, 2009
 *      Author: edgar
 */

#include <stdio.h>

int main(int argc, char *argv[])
{
	if(argc > 1)
		printf("%s", argv[argc-1]);
	return 0;
}
