/*
 * Sample program to illustrate a chain of SPU threads.
 *
 * 6 threads are created
 * Each thread is assigned a different rank from 0 to 5.
 *
 * The PPU sends a signal notification to thread 0.
 * Each thread waits for a signal notification, then do the following:
 * - Thread 0 reads a 4 integer vector, multiplies it by 2 and writes it to
 *   thread 1's local store at the same address (which is the same among all
 *   threads) with dma
 * - Thread n (n=1..5) multiplies the vector in memory (written by thread n-1)
 *   by 2, and sends it to thread n+1 (except thread 5 which writes the result
 *   to main storage)
 *
 * The original vector contains the integers : { 1, 2, 3, 4 } so the result
 * should be { 64, 128, 192, 256 }.
 */

#include <psl1ght/lv2.h>
#include <psl1ght/lv2/spu.h>
#include <lv2/spu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "spu.bin.h"
#include "spustr.h"

#define ptr2ea(x) ((u64)(void *)(x))

int main(int argc, const char* argv[])
{
	sysSpuImage image;
	u32 entry = 0;
	u32 segmentcount = 0;
	sysSpuSegment* segments;
	u32 group_id;
	Lv2SpuThreadAttributes attr = { ptr2ea("mythread"), 8+1, LV2_SPU_THREAD_ATTRIBUTE_NONE };
	Lv2SpuThreadGroupAttributes grpattr = { 7+1, ptr2ea("mygroup"), 0, 0 };
	Lv2SpuThreadArguments arg[6];
	u32 cause, status;
	int i;
	spustr_t *spu = memalign(16, 6*sizeof(spustr_t));
	uint32_t *array = memalign(16, 4*sizeof(uint32_t));

	printf("Initializing 6 SPUs... ");
	printf("%08x\n", lv2SpuInitialize(6, 0));

	printf("Getting ELF information... ");
	printf("%08x\n", sysSpuElfGetInformation(spu_bin, &entry, &segmentcount));
	printf("\tEntry Point: %08x\n\tSegment Count: %08x\n", entry, segmentcount);

	size_t segmentsize = sizeof(sysSpuSegment) * segmentcount;
	segments = (sysSpuSegment*)malloc(segmentsize);
	memset(segments, 0, segmentsize);

	printf("Getting ELF segments... ");
	printf("%08x\n", sysSpuElfGetSegments(spu_bin, segments, segmentcount));

	printf("Loading ELF image... ");
	printf("%08x\n", sysSpuImageImport(&image, spu_bin, 0));

	printf("Creating thread group... ");
	printf("%08x\n", lv2SpuThreadGroupCreate(&group_id, 6, 100, &grpattr));
	printf("group id = %d\n", group_id);

	/* create 6 spu threads */
	for (i = 0; i < 6; i++) {
		spu[i].rank = i;
		spu[i].count = 6;
		spu[i].sync = 0;
		spu[i].array_ea = ptr2ea(array);
		arg[i].argument1 = ptr2ea(&spu[i]);

		printf("Creating SPU thread... ");
		printf("%08x\n", lv2SpuThreadInitialize(&spu[i].id, group_id, i, &image, &attr, &arg[i]));
		printf("thread id = %d\n", spu[i].id);

		printf("Configuring SPU... %08x\n",
		lv2SpuThreadSetConfiguration(spu[i].id, LV2_SPU_SIGNAL1_OVERWRITE|LV2_SPU_SIGNAL2_OVERWRITE));
	}

	printf("Starting SPU thread group... ");
	printf("%08x\n", lv2SpuThreadGroupStart(group_id));

	printf("Initial array: ");
	for (i = 0; i < 4; i++) {
		array[i] = i+1;
		printf(" %d", array[i]);
	}
	printf("\n");

	/* Send signal notification to SPU 0 */
	printf("Sending signal... %08x\n",
		lv2SpuThreadWriteSignal(spu[0].id, 0, 1));

	/* Wait for SPU 5 to return */
	while (spu[5].sync == 0);

	printf("Output array: ");
	for (i = 0; i < 4; i++)
		printf(" %d", array[i]);
	printf("\n");

	printf("Joining SPU thread group... ");
	printf("%08x\n", lv2SpuThreadGroupJoin(group_id, &cause, &status));
	printf("cause=%d status=%d\n", cause, status);

	printf("Closing image... %08x\n", sysSpuImageClose(&image));

	free(array);
	free(spu);

	return 0;
}
