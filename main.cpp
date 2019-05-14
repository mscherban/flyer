#include <iostream>
#include <unistd.h>
#include "Pwm.h"

using namespace std;

int main(int c, char *argv[]) {
	int cnt = 10, i;
	Pwm x("One", "pwm-1:0");
	
	x.set_period(20000);
	x.set_duty_cycle(50);
	x.enable();
	
	while (cnt--) {
		for (i = 0; i < 100; i++) {
			x.set_duty_cycle(i);
			usleep(50000);
		}
	}
	
	return 0;
}