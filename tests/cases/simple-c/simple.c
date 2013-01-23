#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000000

long compute_sum(long* array){
    long sum = 0;
    int i, j;
    
    for(i = 0; i < 250; ++i){
        for(j = 0; j < SIZE; ++j){
            sum += j * i * array[j];
        }
    }
    
    return sum;
}

int main(){
    long* array = (long*) malloc(sizeof(long) * SIZE);

    int i;
    for(i = 0; i < SIZE; ++i){
        array[i] += i;
    }

    long sum = compute_sum(array);

    printf("%ld\n", sum);

    free(array);

    return 0;
}
