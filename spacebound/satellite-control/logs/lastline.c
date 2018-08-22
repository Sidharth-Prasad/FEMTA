#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
void main(void){
	FILE *fhand;
	fhand = fopen("\\logs\\mpu-log.txt", "r");
	char line[1024] ="";
	char c = '1';
	int max_back = -100;
	int i = -2;
	//back up fseek until the newline char is read
	while(i >= max_back && c != '\n'){
		fseek(fhand, i, SEEK_END);
		c = fgetc(fhand);
		//printf("%c", c);
		i--;
		//sleep(1);
	}
	int mpu_read;
	float gx, gy, gz, ax, ay, az, mx, my, mz, quoi;
	fgets(line, sizeof(line), fhand);
	printf("%s", line);
	sscanf(line, "%d %f %f %f %f %f %f %f %f %f %f", &mpu_read, &gx, &gy, &gz, &ax, &ay, &az, &mx, &my, &mz, &quoi);
	//printf("%f\n", gx);
}
