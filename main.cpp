#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "Pwm.h"
#include "BNO055_Imu.h"

using namespace std;

#define IMU_SLEEP_US	((1.0 / IMU_POLLING_RATE_HZ) * 1000000)
#define MOTOR_PWM_FREQ	2100000

#define MOTOR_STEP		3

extern PwmInstance Pwms[];

sem_t input_sem;
sem_t motor_sem;
sem_t hrp_sem;
pthread_mutex_t hrp_mutex;

bool running_g = 1;
int percent_g = 0;

struct {
	float fh, fr, fp;
} Orientation;
float Orientations[3];

float MotorSpeed[4] = { 0 };

/*
iterm: the iterm of the motor
positive: if this motor error (too low) is positive or not
which_orientation : 1 for pitch, 2 for roll
*/
#define ORIENTATION_PITCH	1
#define ORIENTATION_ROLL	2
struct MotorPidInfo {
	float iterm;
	int positive;
	int which_orientation;
	float perror;
};

struct MotorPidInfo MotorPidInfos[4] {
	[FRONT] = {
		.iterm = 0,
		.positive = 1,
		.which_orientation = ORIENTATION_PITCH,
		.perror = 0,
	},
	[RIGHT] = {
		.iterm = 0,
		.positive = 0,
		.which_orientation = ORIENTATION_ROLL,
		.perror = 0,
	},
	[BACK] = {
		.iterm = 0,
		.positive = 0,
		.which_orientation = ORIENTATION_PITCH,
		.perror = 0,
	},
	[LEFT] = {
		.iterm = 0,
		.positive = 1,
		.which_orientation = ORIENTATION_ROLL,
		.perror = 0,
	}
};

#define KP	.5
#define KI	.5
#define KD 	.5

float Pid(int m) {
	float error;
	
	//get the pitch or roll error
	error = Orientations[MotorPidInfos[m].which_orientation];

	//find out if it's actually an error (motor is too low)
	//convert all negative true errors to positive
	if (!MotorPidInfos[m].positive)
		error = -error;

	//if our error is negative here, we are high and don't need to correct
	if (error <= 0.0) {
		return 0;
	}

	//cout << "Motor #" << m << " is error!" << endl;

	return error;
}

/*
	--- STABILITY ---
Pitch:
	Pitch is the angle that the front and back motors make with the earth
	When pitch is:
		Positive: Front is low, back is high
		Negative: Front is high, back is low

	Roll is the angle that the left and right motors make with the earth
	When roll is:
		Positive: Left is low, right is high
		Negative: Left is high, right is low

	Heading is the rotation of the drone, I won't mess with that yet.
 */

void *stabilize(void *n) {
	do {
		sem_wait(&hrp_sem);

		Pid(0);
		Pid(1);
		Pid(2);
		Pid(3);

		MotorSpeed[FRONT] = percent_g;
		MotorSpeed[BACK] = percent_g;




		MotorSpeed[RIGHT] = percent_g;
		MotorSpeed[LEFT] = percent_g;

		sem_post(&motor_sem);
	} while (running_g);

	return 0;
}

void *drive_motors(void *n) {
	Pwm p0(Pwms[FRONT].name, Pwms[FRONT].pwm_dev);
	Pwm p1(Pwms[RIGHT].name, Pwms[RIGHT].pwm_dev);
	Pwm p2(Pwms[BACK].name, Pwms[BACK].pwm_dev);
	Pwm p3(Pwms[LEFT].name, Pwms[LEFT].pwm_dev);

	p0.set_period(MOTOR_PWM_FREQ);
	p1.set_period(MOTOR_PWM_FREQ);
	p2.set_period(MOTOR_PWM_FREQ);
	p3.set_period(MOTOR_PWM_FREQ);

	//set the initial speed to 0
	p0.set_12dc_percent(0);
	p1.set_12dc_percent(0);
	p2.set_12dc_percent(0);
	p3.set_12dc_percent(0);

	p0.enable();
	p1.enable();
	p2.enable();
	p3.enable();

	do {
		//cout << "Setting PWM Percent to: " << percent_g << "%" << endl;
		p0.set_12dc_percent(MotorSpeed[FRONT]);
		p1.set_12dc_percent(MotorSpeed[RIGHT]);
		p2.set_12dc_percent(MotorSpeed[BACK]);
		p3.set_12dc_percent(MotorSpeed[LEFT]);
		sem_wait(&motor_sem); //update with further new data
	} while(running_g);

	p0.disable();
	p1.disable();
	p2.disable();
	p3.disable();

	return 0;
}

void *monitor_imu(void *n) {
	BNO055_Imu Imu;
	HRP_T hrp;
	float ifr, ifp;

	Imu.start();
	usleep(500000); //seems like it needs some time after initial start up

	/* Calibrate for initial IMU orientation */
	hrp = Imu.get_hrp();
	ifr = (float)hrp.r / 16;
	ifp = (float)hrp.p / 16;
	//printf("inital roll: %f, initial pitch: %f\n", ifr, ifp);

	do {
		usleep(IMU_SLEEP_US); //wait for next reading
		hrp = Imu.get_hrp(); //get IMU reading and update orientation
		pthread_mutex_lock(&hrp_mutex);
		Orientation.fh = (float)hrp.h / 16;
		Orientation.fr = ((float)hrp.r / 16) - ifr; //subtract initial error
		Orientation.fp = ((float)hrp.p / 16) - ifp;
		Orientations[ORIENTATION_PITCH] = Orientation.fp;
		Orientations[ORIENTATION_ROLL] = Orientation.fr;
		pthread_mutex_unlock(&hrp_mutex);
		sem_post(&hrp_sem); //tell stabilize thread there is new data
		//printf("heading: %f, roll: %f, pitch: %f\n", Orientation.fh,
							 //Orientation.fr, Orientation.fp);
	} while (running_g);

	return 0;
}

void *monitor_input(void *n) {
	string in;
	int percent = 0;

	usleep(1000000);

	do {
		cout << "Enter PWM Percent (0-100)..." << endl;
		cin >> in;
		
		try {
			//check if a number was input
			percent = stoi(in);
		}
		catch (invalid_argument const &e) {
			//if no number, then check for letters
			if (!in.compare("w")) {
				//"w": increase speed
				percent += MOTOR_STEP;
				percent = min(100, percent);
			} else if (!in.compare("s")) {
				//"s": decrease speed
				percent -= MOTOR_STEP;
				percent = max(0, percent);
			} else {
				//quit on any other letter
				cout << "Stopping..." << endl;
				percent = 0;
				running_g = 0;
			}
		}

		if (percent >= 0 && percent <= 100) {
			cout << "Setting to " << percent << endl;
			percent_g = percent;
			//sem_post(&input_sem);
		}

	} while(running_g);

	return 0;
}

void setup();
void destroy();
int main(int c, char *argv[]) {
	pthread_t imu_thread;
	pthread_t input_thread;
	pthread_t motor_thread;
	pthread_t stab_thread;

	setup();

	//create all threads
	pthread_create(&input_thread, NULL, monitor_input, NULL);
	pthread_create(&imu_thread, NULL, monitor_imu, NULL);
	pthread_create(&stab_thread, NULL, stabilize, NULL);
	pthread_create(&motor_thread, NULL, drive_motors, NULL);
	
	//wait for threads to finish
	pthread_join(imu_thread, NULL);
	pthread_join(input_thread, NULL);
	pthread_join(motor_thread, NULL);
	pthread_join(stab_thread, NULL);

	destroy();
	cout << "Exiting..." << endl;
}

void setup() {
	sem_init(&input_sem, 0, 0);
	sem_init(&motor_sem, 0, 0);
	sem_init(&hrp_sem, 0, 0);
	pthread_mutex_init(&hrp_mutex, NULL);
}

void destroy() {
	pthread_mutex_destroy(&hrp_mutex);
}
