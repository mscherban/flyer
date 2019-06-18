#include <iostream>
#include <vector>
#include <unistd.h>
#include <pthread.h>

#include "Pwm.h"
#include "BNO055_Imu.h"

using namespace std;

extern PwmInstance Pwms[];

bool running = 1;
int engines = 4;

void *monitor_imu(void *n) {
	BNO055_Imu Imu;
	HRP_T hrp;
	float fh, fr, fp;
	float ifr, ifp;

	Imu.start();
	usleep(1000000 / 2);

	hrp = Imu.get_hrp();
	ifr = (float)hrp.r / 16;
	ifp = (float)hrp.p / 16;

	printf("inital roll: %f, initial pitch: %f\n", ifr, ifp);

	while (running) {
		hrp = Imu.get_hrp();
		fh = (float)hrp.h / 16;
		fr = ((float)hrp.r / 16) - ifr;
		fp = ((float)hrp.p / 16) - ifp;
		printf("heading: %f, roll: %f, pitch: %f\n", fh, fr, fp);
		usleep(1000000 / 2);
	}

	return 0;
}

int main(int c, char *argv[]) {
	int percent = 0;
	string in;

	pthread_t t0;

	pthread_create(&t0, NULL, monitor_imu, NULL);

	Pwm p0(Pwms[0].name, Pwms[0].pwm_dev);
	Pwm p1(Pwms[1].name, Pwms[1].pwm_dev);
	Pwm p2(Pwms[2].name, Pwms[2].pwm_dev);
	Pwm p3(Pwms[3].name, Pwms[3].pwm_dev);

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
		cin >> in;
		
		try {
			percent = stoi(in);
		}
		catch (invalid_argument const &e) {

			if (!in.compare("w")) {
				if ((percent += 3) > 100) {
					percent = 100;
				}
			} else if (!in.compare("s")) {
				if ((percent -= 3) < 0) {
					percent = 0;
				}
			}
			else if (!in.compare("a")) {
				cout << "all engines" << endl;
				engines = 4;
				continue;
			}
			else if (!in.compare("f")) {
				cout << "front engine only" << endl;
				engines = 0;
				continue;
			}
			else if (!in.compare("r")) {
				cout << "right engine only" << endl;
				engines = 1;
				continue;
			}
			else if (!in.compare("b")) {
				cout << "back engine only" << endl;
				engines = 2;
				continue;
			}
			else if (!in.compare("l")) {
				cout << "left engine only" << endl;
				engines = 3;
				continue;
			}
			else {
				cout << "Stopping..." << endl;
				break;
			}
		}

		if (percent >= 0 && percent <= 100) {
			cout << "Setting PWM Percent to: " << percent << "%" << endl;
			if (engines == 4) {
				p0.set_12dc_percent((float)percent);
				p1.set_12dc_percent((float)percent);
				p2.set_12dc_percent((float)percent);
				p3.set_12dc_percent((float)percent);
			}
			else if (engines == 0) {
				p0.set_12dc_percent((float)percent);
			}
			else if (engines == 1) {
				p1.set_12dc_percent((float)percent);
			}
			else if (engines == 2) {
				p2.set_12dc_percent((float)percent);
			}
			else if (engines == 3) {
				p3.set_12dc_percent((float)percent);
			}
		}
	} while(1);

	p0.disable();
	p1.disable();
	p2.disable();
	p3.disable();

	running = 0;
	pthread_join(t0, NULL);
	cout << "Exiting..." << endl;
}