#include <stdio.h>

void printAMeans(float A, float B, int N){
    float d = (float)(B - A) / (float)(N + 1);
    printf("%f\n",d);
    for (float i = 1; i <= N; i++) printf("%f ", A + i * d);    
    printf("\n");
    printf("%f, %f, %f\n", A, d, A+d);
    
}

int main(){
    float A = 20.5, B = 32.5;
    int N = 5;
    printAMeans(A, B, N);    
    
    return 0;
}