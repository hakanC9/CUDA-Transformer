#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// function to be integrated
double fun(double x){
  return 1.0/(1.0+x);
}

int main(int argc, char *argv[]){
  if(argc<4){
    printf("Usage: integrate start end partition-divider\n");
    exit(EXIT_FAILURE);
  }
  double start = atof(argv[1]);
  double end = atof(argv[2]);
  unsigned long int partDiv = atol(argv[3]);

  double step = (end-start)/partDiv;
  double accumulator = 0.0;

#pragma omp parallel for shared(start, step, partDiv) reduction(+:accumulator) schedule(auto)
  for(unsigned long int i=0; i<partDiv; i++){
    accumulator += fun(start+i*step);
  }

  double res = accumulator*step;
  printf("Result: %f\n", res);

  return EXIT_SUCCESS;
}
