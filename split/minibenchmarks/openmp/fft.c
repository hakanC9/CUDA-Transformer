#include <complex.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>

#define M_PI 3.1415926535897932384

void fft_rec(double complex* vec, int n){
  if(n<=1)
    return;
  int n2 = n>>1; // n2 = n/2
  // even and odd sub vectors
  double complex *ve = malloc(n2*sizeof(double complex));
  double complex *vo = malloc(n2*sizeof(double complex));
  for(int i=0; i<n2; i++){
    ve[i] = vec[2*i];
    vo[i] = vec[2*i+1];
  }

#pragma omp parallel sections shared(ve, vo, n)
  {
#pragma omp section
    fft_rec(ve, n2);
#pragma omp section
    fft_rec(vo, n2);
  }

  for(int i=0; i<n2; i++){
    double complex w = cos(2*M_PI*i/n)-sin(2*M_PI*i/n)*I;
    vec[i] = ve[i]+w*vo[i];
    vec[n2+i] = ve[i]-w*vo[i];
  }

  free(ve);
  free(vo);
}

int rev(int log2n, int k){    //calculating revers number
  int j, p = 0;
  for(j = 1; j <=log2n; j++) {
    if(k & (1 << (log2n - j))){
      p |= 1 << (j - 1);
    }
  }
  return p;
}


// iterative FFT
void fft(double complex* vec, int n){
  // bit-reverse-copy
  int log2n = (int)log2(n);
#pragma omp parallel for shared(log2n) schedule(auto)
  for(int i=0; i<n; i++){
    int ri = rev(log2n, i);
    if(ri<i){
      double complex buf = vec[i];
      vec[i] = vec[ri];
      vec[ri] = buf;
    }
  }

  /* the following construct:
      for(int kj=0; kj<n/2; kj++){
        int k = (kj/(m/2))*m;
        int j = kj%(m/2);
        ...
     is a direct parallelizable substitusion of:
      for(int k=0; k<n; k+=m){
        for(int j=0; j<m/2; j++){
          ...
  */

  for(int i=1; i<log2n+1; i++){
#pragma omp parallel shared(vec, n, i)
    {
      int m = 2<<(i-1);
      int m2 = m/2;
      int n2 = n/2;
      double complex wm = cexp(-2*M_PI*I/m);
      int j, k;
      double complex t, u, w;
#ifdef PRINT_TH_NO
      int tid = omp_get_thread_num();
      if(tid==0){
	int nthreads = omp_get_num_threads();
	printf("Number of threads = %d\n", nthreads);
      }
#endif
#pragma omp for schedule(auto)
      for(int kj=0; kj<n2; kj++){
	j = kj%m2;
	k = (kj-j)*2;
	w = cpow(wm, j);
	t = w*vec[k+j+m2];
	u = vec[k+j];
	vec[k+j] = u+t;
	vec[k+j+m/2] = u-t;
      }
    } // #pragma parallel
  }
}

int main(int argc, char *argv[]){
  if(argc<2 || argc>3){
    fprintf(stderr, "FFT benchmark for OpenMP,");
    fprintf(stderr, " usage: fft vector-size [repeat-no]\n");
    fprintf(stderr, "vector-size: size of the transformated vector in K of double complex,");
    fprintf(stderr, " must be 2^k, k=1,2...\n");
    fprintf(stderr, "repeat-no: number of repeatitions of the calcolations, default: 1\n");
    exit(EXIT_FAILURE);
  }

  int n = atoi(argv[1])*1024;
  int rpno = argc==3? atoi(argv[2]): 1;

 //  int n = 16;
  double complex *vec = malloc(n*sizeof(double complex));

  for(int i=0; i<n; i++){
    vec[i] = 0.1*i;
  }

  for(int i=0; i<rpno; i++){
    fft(vec, n);
  }

  for(int i=0; i<8; i++){
    printf("%.10e\t%.10ei\n", creal(vec[i]), cimag(vec[i]));
  }

  free(vec);

  return EXIT_SUCCESS;
}
