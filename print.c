#include "print.h"

#define TRUE  0x6L
#define FALSE 0x2L


int64_t p(int64_t val) {
	if((val & 1)){
	    printf("%ld\n", (val-1)/2);
	}
	else if(val == TRUE){
	    printf("true\n");
	}
	else if(val == FALSE){
	    printf("false\n");
	}
	else if(val == 0) {
		printf("null\n");
	}
	else {
	    printf("%ld\n", val);
	}
	return val;
}

void print_heap(int64_t* heap_start, int64_t* heap_end) {
    printf("HEAP_START---------------------------\n");
	int size = 0;

    while(heap_start < heap_end){	
	    printf("-------------------------------------\n");
	    
		//print GC
	    printf("%p  |  GC - ", heap_start);  
        printf("%ld\n", (*heap_start));
	    heap_start++;
	    
		//print size num without change of representation
	    printf("%p  |  Size - ", heap_start);  
		printf("%ld\n", (*heap_start));
		size = (*heap_start);
		heap_start++;

		//print string name with char* cast
 		printf("%p  |  Name = ", heap_start);  
    	char *name = (char *)(*heap_start);
		printf("%s\n", name);
		heap_start++;

		//prints each element in structure changing representation
    	for(int x = 0; x < size ; x++){
	    	printf("%p  |  ",heap_start); printf("[%d] - ", x);  
			p(*heap_start);
	    	heap_start++;
		}
	}	
	printf("HEAP_END------------------------------\n");

}

void print_stack(int64_t* stack_top, int64_t* stack_bottom) {
	fprintf(stderr,"STACK_START========================\n");
	while(stack_top < stack_bottom){
		fprintf(stderr,"%ld\n", (*stack_top));

		stack_top++;
	}
	printf("STACK_END==========================\n");
}
