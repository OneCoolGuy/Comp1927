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
int power_of_two(u_int32_t n);
void vlad_merge(vaddr_t region, vaddr_t adj_region);
int merge_test(vaddr_t link1, vaddr_t link2 , vsize_t size);

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
	int power = 0;
	power = power_of_two(size);
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

void vlad_merge(vaddr_t region, vaddr_t adj_region)
{
	// REGION IS THE REGION THAT IS BEING FREED
	// ADJ REGION IS THE REGION NEXT OT IT THAT IT'S FREE AND IT'S BEING MERGED TO REFION
	free_header_t * region_ptr = point_offset(region); 
	free_header_t * adj_region_ptr = point_offset(adj_region);
	free_header_t * ptr_next = point_offset(adj_region_ptr->next);
	free_header_t * ptr_prev = point_offset(adj_region_ptr->prev);

	if (adj_region_ptr->magic != MAGIC_FREE && adj_region_ptr->magic != MAGIC_ALLOC){
		fprintf(stderr, "Memory corruption");
		abort();
	}
	if (region_ptr->magic != MAGIC_FREE && region_ptr->magic != MAGIC_ALLOC){
		fprintf(stderr, "Memory corruption");
		abort();
	}

	if (region < adj_region){ // check if the region is before or after adj_region
		if (free_list_ptr == adj_region){
			free_list_ptr = (adj_region_ptr->next == free_list_ptr) ? region : adj_region_ptr->next;
			// in case of the adj region that is being merged
		}	// is the only free area 
		ptr_next->prev = adj_region_ptr->prev;
		ptr_prev->next = adj_region_ptr->next;
		region_ptr->size = region_ptr->size << 1; // doubles the region size
		adj_region_ptr->magic = 0xBFFBFFCD;
	} else {
		free_list_ptr = (free_list_ptr == adj_region) ? adj_region_ptr->next : free_list_ptr; // if the adj_region was the free pointer change it to the new region
		ptr_next->prev = adj_region_ptr->prev;
		ptr_prev->next = adj_region_ptr->next;
		adj_region_ptr->size = region_ptr->size << 1; // doubles the region size
		adj_region_ptr->magic = MAGIC_ALLOC;
		region_ptr->magic = 0xBFFBFFCD;
	}
}

void showHeaderInfo(free_header_t* header) 
{
printf("\tChunk index %d, size %d, tag %s, next %d, prev %d\n", (void*)header - (void*)memory, header->size, (header->magic == MAGIC_FREE) ? "FREE" : "ALLOC", header->next, header->prev);
}

int merge_test(vaddr_t link1, vaddr_t link2 , vsize_t size)
{
	vaddr_t start = (link1 > link2) ? link2 : link1;
	if ((memory_size - start) % size == 0 && ((memory_size - (start + size)) % size == 0)){
		return 1;
	}
	return 0;
}

int power_of_two(u_int32_t size)
{
	int count = 0;
	int power = 0;
	int i = 0;
	for (i = 0; i < 31 ; i++){
		if ((size & ( 1 << i)) >> i == 1){
			count++;
			power = i;
		}
	}
	power = (count > 1) ? power + 1 : power;
	return power;
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
	free_header_t * ptr_prev;
	free_header_t * ptr_next;
	void * result = NULL;
	int flag = 0;
	int count = 0;
	int power;
	u_int32_t size;
	vaddr_t link = free_list_ptr;
	vaddr_t allocLink = 0; // link that is supposed tobe the one allocated
	vsize_t allocArea = 0; // size of the allocated area
	vsize_t tempSize = 0; // temporary size used to compare and choose the area to be allocated
	free_header_t * ptr = point_offset(link);

	if (ptr->magic != MAGIC_FREE && ptr->magic != MAGIC_ALLOC){
		fprintf(stderr, "Memory corruption");
		abort();
	}
	
	power = power_of_two(n + sizeof(struct free_list_header));
	size = 1 << power;


	while (link != free_list_ptr || flag == 0){
		if (ptr->magic != MAGIC_FREE){
			return NULL;
		}
		tempSize = (ptr->size >= size) ? ptr->size : tempSize; // changes tempSize to be the the size of the area if it's bigger or equal to size
		if ((tempSize < allocArea || allocArea == 0) && tempSize != 0){
			allocArea = tempSize;
			allocLink = link;
		}
		link = ptr->next;
		ptr = point_offset(link);
		flag = 1;
	}
	if (tempSize == 0){
		return NULL;
	}

	ptr = point_offset(allocLink); //makes ptr to point to the right link
	ptr_next = point_offset(ptr->next); // makes ptr point to the next after the link
	
	while( allocArea > size ) {
		ptr = point_offset(allocLink); //makes ptr to point to the right link
		ptr_next = point_offset(ptr->next); // makes ptr point to the next after the link
		vaddr_t new_addr = allocLink + (ptr->size)/2; 
		header_create((ptr->size)/2,  \
				(ptr->next == allocLink) ? new_addr : ptr->next, \
				(ptr->next == allocLink) ? new_addr : ptr->prev, new_addr , MAGIC_FREE);
		ptr_next->prev = new_addr; 
		ptr->prev = (ptr->prev == allocLink) ? new_addr : ptr->prev;
		ptr->next = new_addr;
		ptr->size = (ptr->size)/2;
		allocArea = ptr->size;
		count++;
	}
	ptr_next = point_offset(ptr->next);
	ptr_prev = point_offset(ptr->prev);
	if ( ptr_prev->prev == allocLink){ // if there are only two regions when the fragmentation started
		ptr_prev->prev = (allocLink + (ptr->size << (count - 1))); // make it point to the first 
	}
	if (ptr == point_offset(ptr->next)){ // if there is only one region to be allocated vlad_malloc should return NULL
		return NULL;
	}
	ptr_prev->next =  ptr->next;
	ptr_next->prev =  ptr->prev;
	ptr->magic = MAGIC_ALLOC;
	result = (void *) point_offset(allocLink + 16); // sets the result to be in the first byte after the header
	ptr = point_offset(free_list_ptr);
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
	free_header_t * ptr_next = NULL; // pointer to the next free memory
	free_header_t * ptr_prev = NULL; // pointer to the previous free meomry
	free_header_t * ptr = (free_header_t *) (object - HEADER_SIZE); // poitner to the region being freed
	free_header_t * temp_ptr = ptr; // pointer used to transverse the list
	vlink_t link_next = 0xACDCACDC;
	vlink_t link_prev = 0xACDCACDC;
	vlink_t link = ((byte *) object - memory - HEADER_SIZE); // memory index to the area being freed
	vlink_t temp_link = link;//momery index used to tranverse the list
	int count = 0; // count to see how far is the other free region;
	int flag;
	if (ptr->magic != MAGIC_FREE && ptr->magic != MAGIC_ALLOC){
		fprintf(stderr, "Memory corruption");
		abort();
	}
	while (((temp_link + temp_ptr->size) < memory_size)){ // loop to find where should 
		temp_link = temp_link + temp_ptr->size; // memory index to the enxt header
		temp_ptr = point_offset(temp_link); // points to the next header
		if (temp_ptr->magic == MAGIC_FREE){
			if (merge_test(link, temp_link, ptr->size *2) && (count == 0) && (temp_ptr->size == ptr->size)){
				vlad_merge(link, temp_link); 
				temp_ptr = ptr;
				temp_link = link;
				count = -1;
			} else {
				ptr_next = temp_ptr; // store the pointer to the next link
				link_next = temp_link; //..
				break;
			}
		}
		count++;
	}
	temp_link = free_list_ptr; // sets the memory index to the first free header
	temp_ptr = point_offset(free_list_ptr);
	if (temp_ptr->magic != MAGIC_FREE && temp_ptr->magic != MAGIC_ALLOC){
		fprintf(stderr, "Memory corruption");
		abort();
	}
	flag = (temp_link > link) ? 0 : 1; // check if the free list starts after the freed region
	count = 0;
	while (flag == 1){
		if(temp_ptr->next > link || (temp_ptr->next == free_list_ptr && count !=0 )){
			if (merge_test(link, link + ptr->size , ptr->size * 2) && point_offset(link + ptr->size) != NULL \
					&& ((point_offset(link + ptr->size)->magic == MAGIC_FREE) && point_offset(link + ptr->size)->size == ptr->size)){
				temp_link = link_next;
				vlad_merge(link, temp_link);
				link_next = (link_next == ptr_next->next) ? free_list_ptr : ptr_next->next;
				ptr_next = point_offset(link_next);
				temp_link = free_list_ptr;
				temp_ptr = point_offset(free_list_ptr);
				count = -1;
			} else if (merge_test(link, temp_link, ptr->size * 2) && temp_link + temp_ptr->size == link && temp_ptr->size == ptr->size){
				vlad_merge(link, temp_link);
				ptr = temp_ptr; // sets pointer to the new header
				link = temp_link;
				temp_link = free_list_ptr; // sets the memory index to the first free header
				temp_ptr = point_offset(free_list_ptr);
				count = -1;
			} else {
				ptr_prev = temp_ptr; // store the prev pointer
				link_prev = temp_link;
				flag = 0;
			}
		}			
		if (count != -1){ // checking if a merge happende
			temp_link = temp_ptr->next;
			temp_ptr = point_offset(temp_link);
		}
		count++;
	}
	if (link_next == link_prev){
		if (ptr_next != NULL && ptr->size != memory_size){
			ptr->prev = (ptr_next->prev == link_prev) ? link_prev : ptr_next->prev; // to make sure that there is only one other free region
			ptr->next = link_next;
			point_offset(ptr->next)->prev = link;
			point_offset(ptr->prev)->next = link;
		} else {
			ptr->prev = link;
			ptr->next = link;
		}
		ptr->magic = MAGIC_FREE;
		free_list_ptr = (ptr->prev < link) ? ptr->prev : link; // check if the previous pointer is smaller then the area being freed
	} else if (link_next == 0xACDCACDC){ // CASE THAT IS NO FREE REGION IN FRONT OF THE FREED REGION
		ptr_next = point_offset(ptr_prev->next);
		ptr->next = ptr_prev->next;
		ptr->prev = link_prev;
		ptr_next->prev = link;
		ptr_prev->next = link;
		ptr->magic = MAGIC_FREE;
	} else if (link_prev == 0xACDCACDC){ // CASE THAT IS NO FREE REGION BEFORE  OF THE FREED REGION
		ptr_prev = point_offset(ptr_next->prev);
		ptr->prev = ptr_next->prev;
		ptr->next = link_next;
		ptr_prev->next = link;
		ptr_next->prev = link;
		ptr->magic = MAGIC_FREE;
		free_list_ptr = link;
	} else { // NORMAL CASE OF A FREE REGION BEFORE AND AFTER THE FREED REGION
		ptr->prev = link_prev;
		ptr->next = link_next;
		ptr_prev->next = link;
		ptr_next->prev = link;
		ptr->magic = MAGIC_FREE;
	}
	/* vlad_stats(); */
}


// Stop the awklocator, so that it can be init'ed again:
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
	// prints all the different partitions
	int flag = 0;
	/* int flag2 = 0; */


	/* assert(point_offset(free_list_ptr)->size == 4096); */
	/* assert(free_list_ptr == 0); */

	free_header_t * ptr = point_offset(free_list_ptr);
	/* free_header_t * ptr2 = (free_header_t *) memory; */
	/* while ( ptr2 != point_offset(free_list_ptr) || flag2 == 0){ */
	/* 	showHeaderInfo(ptr2); */
	/* 	ptr2 = point_offset(ptr2->next); */
	/* 	flag2 = 1; */
	/* } */
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
