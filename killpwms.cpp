#include <iostream>
#include <unistd.h>

#include "Pwm.h"


using namespace std;

extern PwmInstance Pwms[];


int main(int c, char *argv[]) {
	Pwm p0(Pwms[0].name, Pwms[0].pwm_dev);
	Pwm p1(Pwms[1].name, Pwms[1].pwm_dev);
	Pwm p2(Pwms[2].name, Pwms[2].pwm_dev);
	Pwm p3(Pwms[3].name, Pwms[3].pwm_dev);
	
	p0.disable();
	p1.disable();
	p2.disable();
	p3.disable();
}