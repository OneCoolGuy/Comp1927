//
//  COMP1927 Assignment 1 - Vlad: the memory allocator
//  allocator.c ... implementation
//
//  Created by Liam O'Connor on 18/07/12.
//  Modified by John Shepherd in August 2014, August 2015
//  Copyright (c) 2012-2015 UNSW. All rights reserved.
//

#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define HEADER_SIZE    sizeof(struct free_list_header)  
#define MAGIC_FREE     0xDEADBEEF
#define MAGIC_ALLOC    0xBEEFDEAD


/* typedef unsigned int u_int32_t; */
typedef unsigned char byte;
typedef u_int32_t vlink_t;
typedef u_int32_t vsize_t;
typedef u_int32_t vaddr_t;

typedef struct free_list_header {
	u_int32_t magic;  // ought to contain MAGIC_FREE
	vsize_t size;     // # bytes in this block (including header)
	vlink_t next;     // memory[] index of next free block
	vlink_t prev;     // memory[] index of previous free block
} free_header_t;

void header_create(vsize_t size, vlink_t next, vlink_t prev, vaddr_t ptr, int magic);
free_header_t * point_offset(vlink_t offset);
void showHeaderInfo(free_header_t* header);

// Global data

static byte *memory = NULL;   // pointer to start of allocator memory
static vaddr_t free_list_ptr; // index in memory[] of first block in free list
static vsize_t memory_size;   // number of bytes malloc'd in memory[]


// Input: size - number of bytes to make available to the allocator
// Output: none              
// Precondition: Size is a power of two.
// Postcondition: `size` bytes are now available to the allocator
// 
// (If the allocator is already initialised, this function does nothing,
//  even if it was initialised with different size)

void vlad_init(u_int32_t size)
{
	// dummy statements to keep compiler happy
	/* memory = NULL; */
	/* free_list_ptr = (vaddr_t)0; */
	/* memory_size = 0; */
	// TODO
	// remove the above when you implement your code
	int power = 0;
	vsize_t temp;
	while ( temp != 1) {
		temp = size >> power;
		power++;
	}
	if ( power < 9 ){
		memory = malloc(sizeof(byte) * 512);
		free_list_ptr = 0;
		memory_size = 512;
	} else { 
		memory = malloc(sizeof(byte) * size);
		free_list_ptr = 0;
		memory_size = size;
	}
	if (memory == NULL){
		fprintf(stderr, "vlad_init: insufficient memory");
		abort();
	}
	header_create(memory_size, 0, 0, free_list_ptr, MAGIC_FREE);
}


void header_create(vsize_t size, vlink_t next, vlink_t prev, vaddr_t ptr, int magic)
{
	free_header_t * header = point_offset(ptr);
	header->size = size;
	header->next = next;
	header->prev = prev;
	header->magic = magic == MAGIC_FREE ? MAGIC_FREE : MAGIC_ALLOC;
}
	
free_header_t * point_offset(vlink_t offset)
{
	byte * ptr = (byte *) memory ;
	ptr = ptr + offset;
	free_header_t * link = (free_header_t *) ptr;
	return link;
}

void showHeaderInfo(free_header_t* header) 
{
printf("\tChunk index %ld, size %d, tag %s, next %d, prev %d\n", (void*)header - (void*)memory, header->size, (header->magic == MAGIC_FREE) ? "FREE" : "ALLOC", header->next, header->prev);
}

// Input: n - number of bytes requested
// Output: p - a pointer, or NULL
// Precondition: n is < size of memory available to the allocator
// Postcondition: If a region of size n or greater cannot be found, p = NULL 
//                Else, p points to a location immediately after a header block
//                      for a newly-allocated region of some size >= 
//                      n + header size.

void * vlad_malloc(u_int32_t n)
{
	free_header_t * ptr_last;
	free_header_t * ptr_next;
	void * result = NULL;
	int flag = 0;
	int count = 0;
	u_int32_t size = (n + sizeof(free_header_t));
	vaddr_t link = free_list_ptr;
	vaddr_t allocLink = 0;
	vsize_t allocArea = 0;
	vsize_t tempSize = 0;
	free_header_t * ptr = point_offset(link);
	while (link != free_list_ptr || flag == 0){
		if (ptr->magic != MAGIC_FREE){
			abort(); // CHANGE AFTER
		}
		tempSize = (ptr->size > size) ? ptr->size : tempSize;
		if ((tempSize < allocArea || allocArea == 0) && tempSize != 0){
			allocArea = tempSize;
			allocLink = link;
		}
		link = ptr->next;
		ptr = point_offset(link);
		flag = 1;
	}
	if (tempSize == 0){
		printf("NO SIZE\n");
		abort();
	}
	
	while( allocArea >= 2 * size ) {
		showHeaderInfo(ptr);
		showHeaderInfo(point_offset(ptr->next));
		ptr = point_offset(allocLink);
		ptr_next = point_offset(ptr->next);
		vaddr_t new_addr = allocLink + (ptr->size + sizeof(free_header_t))/2;
		header_create(allocLink +(ptr->size - sizeof(free_header_t))/2,  \
				(ptr->next == allocLink) ? new_addr : ptr->next, \
				(ptr->next == allocLink) ? new_addr : ptr->prev, new_addr , MAGIC_FREE);
		if (count >= 1){
			ptr_next->prev = new_addr; 
		}
		ptr->prev = (ptr->prev == allocLink) ? new_addr : ptr->prev;
		ptr->next = new_addr;
		ptr->size = allocLink + (ptr->size - sizeof(free_header_t))/2;
		allocArea = ptr->size;
		count++;
	}
	ptr_next = point_offset(ptr->next);
	ptr_last = point_offset(ptr->prev);
	ptr_last->next =  ptr->next;
	ptr_next->prev =  ptr->prev;
	ptr->magic = MAGIC_ALLOC;
	result = (void *) point_offset(allocLink + 4);
	ptr = point_offset(free_list_ptr);
	showHeaderInfo(ptr);
	showHeaderInfo(point_offset(ptr->next));
	while (ptr->magic != MAGIC_FREE){
		free_list_ptr = ptr->next;
		ptr = point_offset(ptr->next);
		if (ptr == point_offset(free_list_ptr)){
			break;
		}
	}
	return result; // temporarily
}


// Input: object, a pointer.
// Output: none
// Precondition: object points to a location immediately after a header block
//               within the allocator's memory.
// Postcondition: The region pointed to by object can be re-allocated by 
//                vlad_malloc

void vlad_free(void *object)
{
	// TODO
}


// Stop the allocator, so that it can be init'ed again:
// Precondition: allocator memory was once allocated by vlad_init()
// Postcondition: allocator is unusable until vlad_int() executed again

void vlad_end(void)
{
	assert(memory != NULL);
	free(memory);
	memory = NULL;
}



// Precondition: allocator has been vlad_init()'d
// Postcondition: allocator stats displayed on stdout

void vlad_stats(void)
{
	int flag = 0;
	free_header_t * ptr = point_offset(free_list_ptr);
	while (ptr != point_offset(free_list_ptr) || flag == 0){
		showHeaderInfo(ptr);
		ptr = point_offset(ptr->next);
		flag = 1;
	}
	return;
}


//
// All of the code below here was written by Alen Bou-Haidar, COMP1927 14s2
//

//
// Fancy allocator stats
// 2D diagram for your allocator.c ... implementation
// 
// Copyright (C) 2014 Alen Bou-Haidar <alencool@gmail.com>
// 
// FancyStat is free software: you can redistribute it and/or modify 
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or 
// (at your option) any later version.
// 
// FancyStat is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>


#include <string.h>

#define STAT_WIDTH  32
#define STAT_HEIGHT 16
#define BG_FREE      "\x1b[48;5;35m" 
#define BG_ALLOC     "\x1b[48;5;39m"
#define FG_FREE      "\x1b[38;5;35m" 
#define FG_ALLOC     "\x1b[38;5;39m"
#define CL_RESET     "\x1b[0m"


typedef struct point {int x, y;} point;

static point offset_to_point(int offset,  int size, int is_end);
static void fill_block(char graph[STAT_HEIGHT][STAT_WIDTH][20], 
		int offset, char * label);



// Print fancy 2D view of memory
// Note, This is limited to memory_sizes of under 16MB
void vlad_reveal(void *alpha[26])
{
	int i, j;
	vlink_t offset;
	char graph[STAT_HEIGHT][STAT_WIDTH][20];
	char free_sizes[26][32];
	char alloc_sizes[26][32];
	char label[3]; // letters for used memory, numbers for free memory
	int free_count, alloc_count, max_count;
	free_header_t * block;

	/* // TODO */
	/* // REMOVE these statements when your vlad_malloc() is done */
	/* printf("vlad_reveal() won't work until vlad_malloc() works\n"); */
	/* return; */

	// initilise size lists
	for (i=0; i<26; i++) {
		free_sizes[i][0]= '\0';
		alloc_sizes[i][0]= '\0';
	}

	// Fill graph with free memory
	offset = 0;
	i = 1;
	free_count = 0;
	while (offset < memory_size){
		block = (free_header_t *)(memory + offset);
		if (block->magic == MAGIC_FREE) {
			snprintf(free_sizes[free_count++], 32, 
					"%d) %d bytes", i, block->size);
			snprintf(label, 3, "%d", i++);
			fill_block(graph, offset,label);
		}
		offset += block->size;
	}

	// Fill graph with allocated memory
	alloc_count = 0;
	for (i=0; i<26; i++) {
		if (alpha[i] != NULL) {
			offset = ((byte *) alpha[i] - (byte *) memory) - HEADER_SIZE;
			block = (free_header_t *)(memory + offset);
			snprintf(alloc_sizes[alloc_count++], 32, 
					"%c) %d bytes", 'a' + i, block->size);
			snprintf(label, 3, "%c", 'a' + i);
			fill_block(graph, offset,label);
		}
	}

	// Print all the memory!
	for (i=0; i<STAT_HEIGHT; i++) {
		for (j=0; j<STAT_WIDTH; j++) {
			printf("%s", graph[i][j]);
		}
		printf("\n");
	}

	//Print table of sizes
	max_count = (free_count > alloc_count)? free_count: alloc_count;
	printf(FG_FREE"%-32s"CL_RESET, "Free");
	if (alloc_count > 0){
		printf(FG_ALLOC"%s\n"CL_RESET, "Allocated");
	} else {
		printf("\n");
	}
	for (i=0; i<max_count;i++) {
		printf("%-32s%s\n", free_sizes[i], alloc_sizes[i]);
	}

}

// Fill block area
static void fill_block(char graph[STAT_HEIGHT][STAT_WIDTH][20], 
		int offset, char * label)
{
	point start, end;
	free_header_t * block;
	char * color;
	char text[3];
	block = (free_header_t *)(memory + offset);
	start = offset_to_point(offset, memory_size, 0);
	end = offset_to_point(offset + block->size, memory_size, 1);
	color = (block->magic == MAGIC_FREE) ? BG_FREE: BG_ALLOC;

	int x, y;
	for (y=start.y; y < end.y; y++) {
		for (x=start.x; x < end.x; x++) {
			if (x == start.x && y == start.y) {
				// draw top left corner
				snprintf(text, 3, "|%s", label);
			} else if (x == start.x && y == end.y - 1) {
				// draw bottom left corner
				snprintf(text, 3, "|_");
			} else if (y == end.y - 1) {
				// draw bottom border
				snprintf(text, 3, "__");
			} else if (x == start.x) {
				// draw left border
				snprintf(text, 3, "| ");
			} else {
				snprintf(text, 3, "  ");
			}
			sprintf(graph[y][x], "%s%s"CL_RESET, color, text);            
		}
	}
}

// Converts offset to coordinate
static point offset_to_point(int offset,  int size, int is_end)
{
	int pot[2] = {STAT_WIDTH, STAT_HEIGHT}; // potential XY
	int crd[2] = {0};                       // coordinates
	int sign = 1;                           // Adding/Subtracting
	int inY = 0;                            // which axis context
	int curr = size >> 1;                   // first bit to check
	point c;                                // final coordinate
	if (is_end) {
		offset = size - offset;
		crd[0] = STAT_WIDTH;
		crd[1] = STAT_HEIGHT;
		sign = -1;
	}
	while (curr) {
		pot[inY] >>= 1;
		if (curr & offset) {
			crd[inY] += pot[inY]*sign; 
		}
		inY = !inY; // flip which axis to look at
		curr >>= 1; // shift to the right to advance
	}
	c.x = crd[0];
	c.y = crd[1];
	return c;
}
