#include <iostream>
#include <unistd.h>
#include <pthread.h>

#include "Pwm.h"
#include "BNO055_Imu.h"

using namespace std;

typedef struct {
	string name;
	string pwm_dev;
} PwmInstance;

PwmInstance Pwms[4] = {
	[0] = {.name = "One", .pwm_dev = "pwm-1:0"},
	[1] = {.name = "Two", .pwm_dev = "pwm-1:1"},
	[2] = {.name = "Three", .pwm_dev = "pwm-4:0"},
	[3] = {.name = "Four", .pwm_dev = "pwm-4:1"},
};

void *threaded_function(void *n) {
	PwmInstance *pwm = (PwmInstance *)n;
	
	Pwm p(pwm->name, pwm->pwm_dev);
	p.disable();
	p.set_period(5000000);
	p.enable();
	
	p.set_duty_cycle(10);
	sleep(4);
	p.set_duty_cycle(50);
	sleep(4);
	p.set_duty_cycle(30);
	sleep(4);
	p.set_duty_cycle(50);
	sleep(4);
	p.set_duty_cycle(90);
	sleep(4);
	p.set_duty_cycle(80);
	sleep(4);
	p.set_duty_cycle(10);
	sleep(4);
	
	
	return 0;
}

int main(int c, char *argv[]) {
	int running = 50;
	BNO055_Imu Imu;
	HRP_T hrp;
	
	Imu.start();
	while (running--) {
		hrp = Imu.get_hrp();
		printf("heading: %f, roll: %f, pitch: %f\n", hrp.fh, hrp.fr, hrp.fp);
		sleep(1);
	}
	Imu.stop();
	
	// pthread_t t0, t1, t2, t3;
	// Pwm x("One", "pwm-1:0");
	
	// pthread_create(&t0, NULL, threaded_function, (void *)&Pwms[0]);
	// pthread_create(&t1, NULL, threaded_function, (void *)&Pwms[1]);
	// pthread_create(&t2, NULL, threaded_function, (void *)&Pwms[2]);
	// pthread_create(&t3, NULL, threaded_function, (void *)&Pwms[3]);
	
	// x.set_period(20000);
	// x.set_duty_cycle(50);
	// x.enable();
	
	// pthread_join(t0, NULL);
	// pthread_join(t1, NULL);
	// pthread_join(t2, NULL);
	// pthread_join(t3, NULL);
	
	return 0;
}