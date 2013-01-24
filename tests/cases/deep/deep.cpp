#include <iostream>

long abs(long a){
    if(a < 0){
        return -a;
    }

    return a;
}

long compute_third(long a, long b, long c){
    return 1 + ((a * b * c) / (1 + abs(b + 2)));
}

long compute_sum(long a, long b){
    if(a > b){
        return 1 + compute_third(a, b, b) * a * a / (1 + abs(a + b));
    } else {
        return 676 + (compute_third(a, b, b) * a * a * compute_third(b, a, a * b) / (1 + abs(a + b)));
    }
}

template<int N>
long compute(long a){
    if(N == 3){
        return 11 + (compute_sum(N * N * compute<N - 1>(a * a), N * a));
    } else {
        return 99 + (compute_sum(N * N * abs(compute<N - 1>(a + 1)) + compute<N - 1>(a * a), N * a)) * 2;
    }
}

template<>
long compute<0>(long a){
    return 1 + abs(a * a + 99 * a);
}

int main(){
    long sum = 17;
    for(long i = 0; i < 500289; ++i){
        sum += compute<9>(i*i) * compute<10>(i);
    }

    std::cout << sum << std::endl;

    return 0;
}
