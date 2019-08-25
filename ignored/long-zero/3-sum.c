


#include <stdlib.h>
#include <stdio.h>
#include <limits.h>


int main(int argc, char ** args) {
  
  FILE * file = fopen(args[1], "r");

  char buffer[1024];

  double xSum = 0, ySum = 0, zSum = 0;
  double xMax = 0, yMax = 0, zMax = 0;
  double xMin = LONG_MAX, yMin = LONG_MAX, zMin = LONG_MAX;
  
  double lines = 0;

  long errors = 0;
  
  while (fgets(buffer, 1024, file)) {

    double xValue, yValue, zValue;
    sscanf(buffer, "%lf %lf %lf", &xValue, &yValue, &zValue);
    
    xSum += xValue;
    ySum += yValue;
    zSum += zValue;
    
    if (xValue > xMax) xMax = xValue;
    if (yValue > yMax) yMax = yValue;
    if (zValue > zMax) zMax = zValue;
    
    if (xValue < xMin) xMin = xValue;
    if (yValue < yMin) yMin = yValue;
    if (zValue < zMin) zMin = zValue;

    if (xValue > 4.0) errors++;
    if (yValue > 4.0) errors++;
    if (zValue > 4.0) errors++;
    
    lines++;
  }
  
  printf("Sum: %lf, %lf, %lf\n", xSum, ySum, zSum);
  printf("Avg: %lf, %lf, %lf\n", xSum / lines, ySum / lines, zSum / lines);
  printf("Max: %lf, %lf, %lf\n", xMax, yMax, zMax);
  printf("Min: %lf, %lf, %lf\n", xMin, yMin, zMin);

  printf("\nErrors: %ld\n", errors);
  
  return 0;
}
