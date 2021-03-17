/*
 * Copyright (c) 2010, Stefan Lankes, RWTH Aachen University
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0, <LICENSE-APACHE or
 * http://apache.org/licenses/LICENSE-2.0> or the MIT license <LICENSE-MIT or
 * http://opensource.org/licenses/MIT>, at your option. This file may not be
 * copied, modified, or distributed except according to those terms.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define N	255

/* For the write_asm function, the C variables used inside the asm()
   construct need to have global scope. */
volatile size_t fd_g = 0;
volatile char *buf_g;
volatile size_t len_g;
volatile ssize_t ret_g;


static ssize_t write_asm(int fd, char *buf)
{
	/* Take arguments and assign them to globally declared variables
	   so we can use them in the asm() construct */
	fd_g = fd;
	buf_g = buf;
	len_g = strlen(buf);

	asm volatile("mfence":::"memory");
	asm volatile  (	"mov	$1, %%rax\n\t"
			"mov	fd_g, %%rdi\n\t"
			"mov	buf_g, %%rsi\n\t"
			"mov 	len_g, %%rdx\n\t"
			"syscall\n\t"
			"mov    %%rax, ret_g\n\t"
			:
			:
			: "%rax", "%rdi", "%rsi", "%rdx"
			);
	
	return ret_g;
}

static ssize_t printf_asm(char *buf)
{
	return write_asm(1, buf);
}

static void syscall_tester(void)
{
	ssize_t ret;

	ret = printf_asm("Hello World from the assembler!!\n");
	printf("Returned from first syscall\n");
	printf("%zd values were printed\n\n", ret);

	ret = printf_asm("Hello once again from the assembler!!\n");
	printf("Returned from second syscall\n");
	printf("%zd values were printed\n\n", ret);

	//ret = write_asm(1, "sadf\n");
	//printf_asm("jahskjdhfashf\n");
}

int main(int argc, char** argv)
{
	int i, random;
	FILE* file;

	printf("Hello World!!!\n");
	for(i=0; environ[i]; i++)
		printf("environ[%d] = %s\n", i, environ[i]);
	for(i=0; i<argc; i++)
		printf("argv[%d] = %s\n", i, argv[i]);

	file = fopen("/etc/hostname", "r");
	if (file)
	{
		char fname[N] = "";

		fscanf(file, "%s", fname);
		printf("Hostname: %s\n", fname);
		fclose(file);
	} else fprintf(stderr, "Unable to open file /etc/hostname\n");

	file = fopen("/tmp/test.txt", "w");
	if (file)
	{
		fprintf(file, "Hello World!!!\n");
		fclose(file);
	} else fprintf(stderr, "Unable to open file /tmp/test.txt\n");

	syscall_tester();

	return 0;
}
