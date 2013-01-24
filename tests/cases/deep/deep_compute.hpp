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
