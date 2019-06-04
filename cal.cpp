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
	int percent;

	p0.set_period(2100000);
	p1.set_period(2100000);
	p2.set_period(2100000);
	p3.set_period(2100000);

	p0.set_12dc_percent(0);
	p1.set_12dc_percent(0);
	p2.set_12dc_percent(0);
	p3.set_12dc_percent(0);

	p0.enable();
	p1.enable();
	p2.enable();
	p3.enable();
	do {
		cout << "Enter PWM Percent (0-100)..." << endl;
		cin >> percent;
		if (percent >= 0 && percent <= 100) {
			cout << "Setting PWM Percent to: " << percent << "%" << endl;
			p0.set_12dc_percent((float)percent);
			p1.set_12dc_percent((float)percent);
			p2.set_12dc_percent((float)percent);
			p3.set_12dc_percent((float)percent);
		}
		else {
			break;
		}
	} while(1);

	p0.disable();
	p1.disable();
	p2.disable();
	p3.disable();
}