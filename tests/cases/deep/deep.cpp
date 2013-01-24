#include <iostream>

#include "deep_compute.hpp"
#include "deep_sum.hpp"

int main(){
    long sum = 17;
    for(long i = 0; i < 500289; ++i){
        sum += compute<9>(i*i) * compute<10>(i);
    }

    std::cout << sum << std::endl;

    return 0;
}
