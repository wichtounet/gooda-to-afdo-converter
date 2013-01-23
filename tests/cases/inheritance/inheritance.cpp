#include <iostream>

#define SIZE 10000000

struct A {
    long a = 0;

    long compute_sum(int i, long* array){
        long sum = 0;
        for(int j = 0; j < SIZE; ++j){
            sum += compute_sum(i, j, array[j]);
        }
        a++;
        return sum;
    }

    long compute_sum(int i, int j, long v){
        return a * ((i * j) + (j * j * v)) / (v + 1);
    }
};

int main(){
    A a;
    
    long* array = new long[SIZE];
    long sum = 0;

    for(int i = 0; i < SIZE; ++i){
        array[i] += i;
    }
    
    for(int i = 0; i < 500; ++i){
        sum += a.compute_sum(i, array);
    }

    std::cout << sum << std::endl;
    std::cout << a.a << std::endl;

    delete[] array;

    return 0;
}
