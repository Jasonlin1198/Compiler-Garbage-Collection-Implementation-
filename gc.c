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
	    if((((Data*)(*obj))->gc_metadata != 0) && (((Data*)(*obj))->gc_metadata) == 1){
			//tag on forward address number to gc 
  			((Data*)(*obj))->gc_metadata = ((Data*)(*obj))->gc_metadata | (*obj_to);
			//move to pointer to next space
			size_to = ((Data*)(*obj_to))->size;
			obj_to += size_to + 3; 
		}
		//move heap pointer forward to next obj until at last marked obj in stack
		size_from = ((Data*)(*obj))->size;
		obj += size_from + 3; 
		
	}
	return;	
}

//traverse and update all addresses
void forward2(int64_t* heap_start, int64_t* max){
	int size = 0;
	int64_t* start = heap_start;
	int64_t* max_add = max;

	int64_t* newObj;

	//loop through heap
	while(start < max_add){
		//check if marked obj
		if((((Data*)(*start))->gc_metadata != 0) && ((((Data*)(*start))->gc_metadata) & LSB) == 1) {
			for(int i = 0; i < ((Data*)(*start))->size; i++){
				//check if elem is a reference to another obj
				if(((((Data*)(((Data*)(*start))->elements[i]))->gc_metadata & 7L) == 0) && (((Data*)(((Data*)(*start))->elements[i]))->gc_metadata != 0)){
					//set new address to be original's first word with removed marked bit ------------------           & operator for element???
				 	newObj = &(((Data*)(*start))->elements[i]);
					((Data*)((Data*)(*start))->elements[i])->gc_metadata = (((Data*)(*newObj))->gc_metadata) & TAG;
				}
			}
		}
		start += (((Data*)(*start))->size) + 3;
	}
}

//moves all live data to correct position -- uses forward pointer to put in spot
int64_t* compact(int64_t* move_from, int64_t* move_to, int64_t* max_add){

	//initialize heap pointers
	int64_t* obj = move_from;
	int64_t* obj_to = move_to;
	int64_t* max = max_add;

	int size_to;
	int size_from;

	//loop for heap
	while(obj < max){
		//if heap obj is marked
	    if((((Data*)(*obj))->gc_metadata != 0) && ((((Data*)(*obj))->gc_metadata) == 1)){

			((Data*)(*obj_to))->gc_metadata = 0;
			((Data*)(*obj_to))->size = ((Data*)(*obj))->size;
			((Data*)(*obj_to))->name = ((Data*)(*obj))->name;
			for(int i = 0; i < ((Data*)(*obj))->size; i++){
				((Data*)(*obj_to))->elements[i] = ((Data*)(*obj))->elements[i];
			}
			//move to pointer to next space
			size_from = ((Data*)(*obj))->size;
			obj_to += size_from + 3; 
	
			obj += size_from + 3;
	
		}
		else{
			//move heap pointer forward to next obj until at last marked obj in stack
			size_from = ((Data*)(*obj))->size;
			obj += size_from + 3; 
	
		}
			
	}
	
	return obj_to;
}

/*
void compact(int64_t* f, int64_t* t, int64_t* stack_top, int64_t* max){
	int size = 0;
	int size_to = 0;

	//iteration pointer: f
	//placement pointer: t
	Data* max_add = (Data*)(*max);
	while(f, max_add){
		//marked
		if(((f->gc_metadata) & LSB) == 1){
			//if gc forward matches original hex address
			if((((f->gc_metadata) & LSB)) == (*from)){
			
			}
			else{
				//updates address to compact
				t = f;
				size_to = t->size;
				t += size_to + 3;
			
			}
			//move forward iteration pointer
		

			//clear gc data
			f->gc_metadata = 0;
		}
			size = f->size;	
			f += size + 3;
	}	
}
*/
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
	fprintf(stderr, "This is first frame : %ld",*first_frame);

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
	while(first_frame < stack_top){
		//check if value in stack is a number address
		if((first_frame != NULL) && ((*first_frame & 7L) == 0)){
			max_add = (int64_t*)(*first_frame);
			mark_heap(*first_frame);
		}
		first_frame--;
	}



	forward1(move_to, move_from, max_add);

	forward2(start,max_add);

	alloc_ptr = compact(from,to, max_add);
    return alloc_ptr;
}
