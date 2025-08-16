#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

static int SIZE;

void calculateIteration(double *constTemp, double *inputTemp, double *outputTemp, double speed);
void printResults(double *temp);

int main(int argc, char *argv[]){
  if(argc<3){
    printf("Usage: heat SIZE step-no speed\n");
    exit(EXIT_FAILURE);
  }
  SIZE = atoi(argv[1]);
  int stepNo = atoi(argv[2]);
  double speed = atof(argv[3]);

  // create buffer with heaters
  double *constTemp = calloc(SIZE*SIZE, sizeof(double));
  // insert a heater into constTemp in loc: 0,0
  constTemp[0] = 1.0;

  // buffers with calculated temps
  double *temp1 = calloc(SIZE*SIZE, sizeof(double)); // inited with 0s
  double *temp2 = malloc(SIZE*SIZE*sizeof(double));  // to be overriden anyway
  double *temps[] = {temp1, temp2};

  for(int i=0; i<stepNo; i++){
    calculateIteration(constTemp, temps[0], temps[1], speed);
    // temps[] swap
    temp1 = temps[0];
    temps[0] = temps[1];
    temps[1] = temp1;
  }

  if ((argc == 5) && !strncmp(argv[4], "--notrace", 9)) {
    return EXIT_SUCCESS;
  } else {
    printResults(temps[0]);
  }
  return EXIT_SUCCESS;
}

void printResults(double *temp){
  for(int i=0; i<30; i++){
    for(int j=0; j<64; j++){
      double val = temp[i*SIZE+j];
      printf("%c", val>1.0||val<0.0? '!': (char)('0'+val*10));
    }
    printf("\n");
  }
}


void calculateIteration(double *constTemp, double *inputTemp, double *outputTemp, double speed){
#pragma omp parallel for shared(constTemp, inputTemp, outputTemp, speed) schedule(auto)
  for(int y=0; y<SIZE; y++){
    int offset = y*SIZE;
    for(int x=0; x<SIZE; x++){
      if(constTemp[offset] > 0.0){
	outputTemp[offset] = constTemp[offset];
      }else{
	int loff = (x>0? offset-1: offset);
	int roff = (x<SIZE-1? offset+1: offset);
	int toff = (y>0? offset-SIZE: offset);
	int boff = (y<SIZE-1? offset+SIZE: offset);
	double inp = constTemp[offset]>0.0? constTemp[offset]: inputTemp[offset];
	double left = constTemp[loff]>0.0? constTemp[loff]: inputTemp[loff];
	double right = constTemp[roff]>0.0? constTemp[roff]: inputTemp[roff];
	double top = constTemp[toff]>0.0? constTemp[toff]: inputTemp[toff];
	double bottom = constTemp[boff]>0.0? constTemp[boff]: inputTemp[boff];
	outputTemp[offset]= inp + speed*(left+right+top+bottom-4*inp);
      }
      offset++;
    }
  }
}
