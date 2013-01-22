#include <iostream>

#define SIZE 10000000

long compute_sum(long* array){
    long sum = 0;
    for(int i = 0; i < 250; ++i){
        for(int j = 0; j < SIZE; ++j){
            sum += j * i * array[j];
        }
    }
    return sum;
}

int main(){
    long* array = new long[SIZE];

    for(int i = 0; i < SIZE; ++i){
        array[i] += i;
    }

    long sum = compute_sum(array);

    std::cout << sum << std::endl;

    delete[] array;

    return 0;
}
