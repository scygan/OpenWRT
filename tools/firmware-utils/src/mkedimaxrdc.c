/*
 * Copytight (C) 2008  Slawomir Cygan <slawomir.cygan[at]gmail.com>
 * Copyright (C) 2004  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


/* September 5, 2008
 *
 *	This is image generator for BR-6315SRG router based on trx.c code by Manuel Novoa III  <mjn3@codepoet.org>
 *  Added pad on the begin o the image, variables for the bootloader script and many others..
 *
 * November 11, 2008
 *  Added support for BR-6215SRG router
 *  Support for web images
 *  TRX header is now optional (and obsolete, due to my kernel changes)
 *
 * August 28, 2011
 *  Deleted support for trx
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define _GNU_SOURCE
#include <getopt.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define STORE32_LE(X)		bswap_32(X)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define STORE32_LE(X)		(X)
#else
#error unkown endianness!
#endif

uint32_t crc32buf(char *buf, size_t len);

typedef struct 
{
	char * name ; 
	int  maxSize;
	int  startPad;
	char * modelCode; 
} boardInfo;

boardInfo boards[]=
{ 
	{"BR-6315SRG",0x7E0000,0x8000, "WRRM"}, /*8MB - 128K for bootloader*/ 
	{"BR-6215SRG",0x3E0000,0x8000, "WRRM"},
	{"3G-6200WG",0x3E0000,0x8000, "3GR1"}, 
	{"3G-6200N",0x3D0000,0x0, "3G62"}, /*4MB - 4 * 64K for bootloader*/ 
	{0,0}
};


struct option long_options[] = 
{
	{"board",required_argument,0,'b'},
	{"help",no_argument,0,'h'},
	{"output",required_argument,0,'o'},
	{"kernel",required_argument,0,'k'},
	{"file",required_argument,0,'f'},
	{"align",required_argument,0,'a'},
	{"append",required_argument,0,'A'},
	{"web",no_argument,0,'w'},
	{"trx",no_argument,0,'t'},
	{0,0,0,0}
};

struct __attribute((packed)) csysHeader
{
	char	 csys[4]; 
	uint32_t linuxLength;
	uint32_t initrdLength;
	char	 modelCode[4]; 
	uint32_t unknown3; /* Edimax Web application takes size for checksum from here */
} ;

struct csysHeader * cHdr; 


/**********************************************************************/

void usage(void) __attribute__ (( __noreturn__ ));

void usage(void)
{
	fprintf(stderr, "Usage: mkedimaxrdc -b board [-t] [-w] [-k file] -o outfile [-a align] [-f file] [-f file [-f file]] [-A file [-A file]]\n");
	exit(EXIT_FAILURE);
}

void printHelp(void)
{
	int i=0;
	printf("Help:\nSupported options: \n \t --board (-b) \t\tBoard type\n\t --kernel (-k)\t\tKernel bzImage file\n\t --output (-o)\t\tOutput file\n\t--align (-a)\t\t Align next image\n\t--append (-A)\t\timage file without generating trx info\n\t--file (-f)\t\tImage file\n\t--web (-A)\t\tgenerate image for webui\n");
	printf("Supported boards :");
	while (boards[i].name)
	{
		printf(" %s",boards[i].name);
		i+=1;
	}
	printf(".\n");
}

int fileSize(char * name ) 
{
	struct stat buf;
	
	if (lstat(name,&buf)==0)
		return buf.st_size;
	else
	{
		perror("lstat: " );
		exit(3);
	}
}

int main(int argc, char **argv)
{
	FILE *out = stdout;
	FILE *in;
	char *ofn = NULL;
	char *buf;
	char *e;
	int c, i, append = 0,boardIdx=-1, option_index=0;
	size_t n;
	uint32_t cur_len;
	char web=0;
  int maxlen;

	fprintf(stderr, "Image generator for RDC based Edimax (C) Slawomir Cygan 2008 <slawomir.cygan@gmail.com>\n");

	in = NULL;
	i=0;

	/*prepare*/

	while ((c = getopt_long(argc, argv, "o:a:b:f:A:hk:wt",long_options,&option_index)) != -1)
	{
		switch(c)
		{
			case 'b':
				if (boardIdx!=-1)
					usage();
				boardIdx=0;
				while ((boards[boardIdx].name)&&(strcmp(optarg,boards[boardIdx].name)!=0))
					boardIdx+=1;
				if (!boards[boardIdx].name)
				{
					fprintf(stderr,"Board unsupported. Try -h to get list of supported boards\n");
					return EXIT_FAILURE;
				}
				break;
			case 'h':
				printHelp();
				return 0;
				break;
			case 'w' : 
				web=1;
				break;
			case 'A':
			case 'f':
			case 1:
			case 'o':
			case 'a':
			case 'k':
				break;
			default:
				usage();
		}
	}

	if (boardIdx==-1)
	{
		fprintf(stderr,"Board is not set. Try -h to get liust of supported boards\n");
		return  EXIT_FAILURE;
	}

	maxlen=boards[boardIdx].maxSize;
	if (!(buf = malloc(boards[boardIdx].maxSize))) {
		fprintf(stderr, "malloc failed\n");
		return EXIT_FAILURE;
	};

	cur_len=boards[boardIdx].startPad;
	
	i=0;
	option_index=0;
	optind=0;

	cHdr = (struct csysHeader * ) &buf[cur_len];
	bzero(cHdr,sizeof(struct csysHeader));

	while ((c = getopt_long(argc, argv, "o:a:f:A:hk:twb:",long_options,&option_index)) != -1) {
		switch (c) {
			
			case 'k':
				if (cur_len!=boards[boardIdx].startPad)
				{
					fprintf(stderr,"Error: you should place kernel on the beginning\n");
					return EXIT_FAILURE;
				}
				
				cHdr->initrdLength=STORE32_LE(1); /* i dont't use initrd, but it wont't boot with 0 length*/
				cHdr->linuxLength=STORE32_LE(fileSize(optarg));
				strncpy(cHdr->csys,"CSYS",4);
				strncpy(cHdr->modelCode,boards[boardIdx].modelCode,4);
				cHdr->unknown3 = 0;  /* size for checksum check set to 0, cause i don't know how to calclate it */

				memcpy(buf+cur_len,cHdr,sizeof(struct csysHeader));
				cur_len+=sizeof(struct csysHeader);

				if (!(in = fopen(optarg, "r"))) {
					fprintf(stderr, "can not open \"%s\" for reading\n", optarg);
					usage();
				}
				n = fread(buf + cur_len, 1, maxlen - cur_len, in);
				if (!feof(in)) {
					fprintf(stderr, "fread failure or file \"%s\" too large\n",optarg);
					fclose(in);
					return EXIT_FAILURE;
				}
				fclose(in);
#undef  ROUND
#define ROUND 4
				if (n & (ROUND-1)) {
					memset(buf + cur_len + n, 0, ROUND - (n & (ROUND-1)));
					n += ROUND - (n & (ROUND-1));
				}
				cur_len += n;
				append = 0;

				break;

			case 'A':
				append = 1;
				/* fall through */


			case 'f':
				if (!append)
				{
					if (cHdr->initrdLength==1)
						cHdr->initrdLength=STORE32_LE(cur_len);
				}

				if (!(in = fopen(optarg, "r"))) {
					fprintf(stderr, "can not open \"%s\" for reading\n", optarg);
					usage();
				}
				n = fread(buf + cur_len, 1, maxlen - cur_len, in);
				if (!feof(in)) {
					fprintf(stderr, "fread failure or file \"%s\" too large\n",optarg);
					fclose(in);
					return EXIT_FAILURE;
				}
				fclose(in);
#undef  ROUND
#define ROUND 4
				if (n & (ROUND-1)) {
					memset(buf + cur_len + n, 0, ROUND - (n & (ROUND-1)));
					n += ROUND - (n & (ROUND-1));
				}
				cur_len += n;
				append = 0;

				break;
			case 'o':
				ofn = optarg;
				if (ofn && !(out = fopen(ofn, "w"))) {
					fprintf(stderr, "can not open \"%s\" for writing\n", ofn);
					usage();
				}

				break;
			case 'a':
				errno = 0;
				n = strtoul(optarg, &e, 0);
				if (errno || (e == optarg) || *e) {
					fprintf(stderr, "illegal numeric string\n");
					usage();
				}
				if ((cur_len) & (n-1)) {
					n = n - ((cur_len) & (n-1));
					memset(buf + cur_len, 0, n);
					cur_len += n;
				}
				break;
			case 'h' :
			case 't' :
			case 'w' :
      case 'b' :
				break;
			default:
				usage();
		}
	}

	if (!in) {
		fprintf(stderr, "we require atleast one filename\n");
		usage();
	}

#undef  ROUND
#define ROUND 0x1000
	n = cur_len & (ROUND-1);
	if (n) {
		memset(buf + cur_len, 0, ROUND - n);
		cur_len += ROUND - n;
	}

	if (web)
	{
		cur_len-=boards[boardIdx].startPad;
		buf+=boards[boardIdx].startPad;
	}
	if (!fwrite(buf, cur_len, 1, out) || fflush(out)) {
		fprintf(stderr, "fwrite failed\n");
		return EXIT_FAILURE;
	}

	fclose(out);
	return EXIT_SUCCESS;
}

