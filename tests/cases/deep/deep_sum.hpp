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
