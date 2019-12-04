#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "print.h"
#include "gc.h"

// STUDENT: Change this to 1 in order to print
#define DEBUG 0

#define DEBUG_PRINT(...) \
     do { if (DEBUG) fprintf(stdout, __VA_ARGS__); } while (0)

#define LSB 0x0000000000000001L
#define TAG 0x1111111111111110L

typedef struct {
  int64_t gc_metadata;
  int64_t size;
  char* name;
  int64_t elements[];
} Data;

void mark_heap(int64_t frame){
	//already marked
	Data* d1 = (Data*)frame;
	if((d1->gc_metadata) == 1){
		return; 
	}		
	//set gc val to marked
	d1->gc_metadata = 1;
	int size = d1->size;
	//iterate through size of heap obj elems
	for(int x = 0; x < size; x++){
		if((d1->elements[x] & 7L) == 0){
			mark_heap(d1->elements[x]);
		}
	}		
	return;
}

void forward1(int64_t* move_from, int64_t* move_to, int64_t* max_add){

	//initialize heap pointers
	int64_t* obj = move_from;
	int64_t* obj_to = move_to;
	int64_t* max = max_add;

	int size_to;
	int size_from;

	//loop for heap
	while(obj < max){
		//if heap obj is marked
	    if((obj != 0) && (obj_to != 0) && (((Data*)(obj))->gc_metadata == 1)){
			//tag on forward address number to gc 
  			((Data*)(obj))->gc_metadata = ((Data*)(obj))->gc_metadata | (int64_t) (obj_to);
			//move to pointer to next space
			size_to = ((Data*)(obj_to))->size;
			obj_to += size_to + 3; 
		}
		//move heap pointer forward to next obj until at last marked obj in stack
		size_from = ((Data*)(obj))->size;
		obj += size_from + 3; 
		
	}
	return;	
}

//traverse and update all addresses
void forward2(int64_t* heap_start, int64_t* max){
	int size = 0;
	int64_t* start = heap_start;
	int64_t* max_add = max;
	Data* newObj;

	//loop through heap
	while(start < max_add){
		//check if marked obj
		if((start != 0) && (((Data*)(start))->gc_metadata & LSB) == 1){
			for(int i = 0; i < ((Data*)(start))->size; i++){
				//check if elem is a reference to another obj
				if(((((Data*)(((Data*)(start))->elements[i]))->gc_metadata & 7L) == 0) && (((Data*)(((Data*)(start))->elements[i])) != 0)){
					//set new address to be original's first word with removed marked bit ------------------           & operator for element???
				 	newObj = (Data*)(((Data*)(start))->elements[i]);
					((Data*)((Data*)(start))->elements[i])->gc_metadata = (newObj->gc_metadata) & TAG;
				}
			}
		}
		start += (((Data*)(start))->size) + 3;
	}
}

//moves all live data to correct position -- uses forward pointer to put in spot
int64_t* compact(int64_t* move_from, int64_t* move_to, int64_t* stack, int64_t* top,  int64_t* max_add){

	//initialize heap pointers
	int64_t* obj = move_from;
	int64_t* obj_to = move_to;
	int64_t* max = max_add;
	int size_from;
	int64_t* stack_copy = stack;

	fprintf(stderr, "obj is : %p\n", obj);

	fprintf(stderr, "obj_to is : %p\n", obj_to);
	//loop for heap
	while(obj < max){
		//if heap obj is marked

	    if((obj != 0) && (obj_to != 0) && ((((Data*)(obj))->gc_metadata) == 1)){
			size_from = ((Data*)(*obj))->size;
			//loop through stack		
			while(stack > top){
				//update stack references if stack obj address and heap address is the same
			    	//if(int64 stack address == int64 heap address)
				if((*stack) == (obj)){
					*stack = obj_to;
				}
				stack--;
			}
			//reset bottom pointer
			stack = stack_copy;
			
	
			((Data*)(obj_to))->gc_metadata = 0;
			((Data*)(obj_to))->size = ((Data*)(obj))->size;
			((Data*)(obj_to))->name = ((Data*)(obj))->name;
			for(int i = 0; i < ((Data*)(obj))->size; i++){
				((Data*)(obj_to))->elements[i] = ((Data*)(obj))->elements[i];
			}
			//move to pointer to next space */
			obj += size_from + 3;
			obj_to += size_from + 3; 
		}
		else{
			//move heap pointer forward to next obj until at last marked obj in stack
			size_from = ((Data*)(obj))->size;
			obj += size_from + 3; 
		}
	}
	return obj_to;
}

int64_t* gc(int64_t* stack_bottom,
            int64_t* first_frame,
            int64_t* stack_top,
            int64_t* heap_start,
            int64_t* heap_end,
            int64_t* alloc_ptr) {
     DEBUG_PRINT("starting GC...\n");
     DEBUG_PRINT("\tstack top    = 0x%p\n\tstack_bottom = 0x%p\n\tfirst_frame  = 0x%p\n\theap start   = 0x%p\n\theap_end     = 0x%p\n",
                 stack_top,
                 stack_bottom,
                 first_frame,
                 heap_start,
                 heap_end);

	//move pointer past return pointers in stack
	first_frame -= 2;
	
	int64_t* max_add;
	int64_t* move_to;
	int64_t* move_from;
	int64_t* start;
	int64_t* from;
	int64_t* to;

	//initialize forwarding pointers to valid heap search
	max_add = heap_start;
	move_to = heap_start;
	move_from = heap_start;

	start = heap_start;
	
	from = heap_start;
	to = heap_start;

	//check references to the stack for heap objects
	while(first_frame > stack_top){
		//check if value in stack is a number address
		if((first_frame != 0) && ((*first_frame & 7L) == 0)){
			max_add = (int64_t*)(*first_frame);
			mark_heap(*first_frame);
		}
		first_frame--;
	}

	forward1(move_to, move_from, max_add);

	forward2(start,max_add);


	int64_t* stack = first_frame;
	int64_t* top = stack_top;
	alloc_ptr = compact(from,to, stack, top, max_add);
    return alloc_ptr;
}
