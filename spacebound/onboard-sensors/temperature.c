#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define RESET   "\033[0m"
#define RED     "\033[31m"

int main() {

  FILE *temperatureFile;
  double T;
  
  for (;;) {

    temperatureFile = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

    // Upon failure
    if (temperatureFile == NULL) {
      printf(RED "The onboard temperature probe could not be read" RESET);
      exit(1);
    }

    fscanf(temperatureFile, "%lf", &T);
    printf("\rTemperature: %6.3f °C.", T / 1000);
    fflush(stdout);

    fclose(temperatureFile);

    sleep(1);
  }

  printf("\n");
  return 0;
}
