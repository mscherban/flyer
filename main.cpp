#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#include "Pwm.h"
#include "BNO055_Imu.h"

using namespace std;

#define IMU_SLEEP_US	((1.0 / IMU_POLLING_RATE_HZ) * 1000000)
#define MOTOR_PWM_FREQ	2100000

#define MOTOR_STEP		3

#define DEBUG_FILENAME	"debug.csv"
fstream fdebug;

void print_2_csv();

extern PwmInstance Pwms[];

bool running_g = 1;
int percent_g = 0;
short Orientations[3] = { 0 };
short MotorSpeed[4] = { 0 };

struct MotorPidDebug {
	short pterm, iterm, dterm;
	short pid;
	short error;
	short motorspeed;
};

struct {
	short setspeed;
	short imu_h, imu_r, imu_p;
	struct MotorPidDebug Motor[4];
} DebugInfo;

/*
iterm: the iterm of the motor
positive: if this motor error (too low) is positive or not
which_orientation : 1 for pitch, 2 for roll
perror: previous error
*/
#define ORIENTATION_HEADING	0
#define ORIENTATION_PITCH	1
#define ORIENTATION_ROLL	2
struct MotorPidInfo {
	short iterm;
	int positive;
	int which_orientation;
	short perror;
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

#define MAX_SPEED_PID_BOOST	5
#define KP	0.1
#define KI	0.0
#define KD 	0.0

int Pid(int m) {
	int error;
	int pterm, iterm, dterm, pid;
	
	//get the pitch or roll error
	error = Orientations[MotorPidInfos[m].which_orientation];

	//find out if it's actually an error (motor is too low)
	//convert all negative true errors to positive
	if (!MotorPidInfos[m].positive)
		error = -error;

	//if our error is negative here, we are high and don't need to correct
	//if (error <= 0.0) {
	//	return 0;
	//}

	//pterm is error, later multiplied by coefficient
	pterm = error;

	//iterm is the integration of previous errors
	iterm = MotorPidInfos[m].iterm + (error * (1.0f/IMU_POLLING_RATE_HZ));

	//dterm is how quickly it changed from previous error
	dterm = error - MotorPidInfos[m].perror;

	//sum and multiply by coefficients for the control value
	pid = (pterm * KP) + (iterm * KI) - (dterm * KD);

 	DebugInfo.Motor[m].pterm = pterm;
	DebugInfo.Motor[m].iterm = iterm;
	DebugInfo.Motor[m].dterm = dterm;
	DebugInfo.Motor[m].pid = pid;
	DebugInfo.Motor[m].error = error; 

	//update for next go
	MotorPidInfos[m].perror = error;
	MotorPidInfos[m].iterm = iterm;

	//if (error <= 0.0) {
	//	pid = 0;
	//}

	//restrict the pid to a maximum of 10% above setspeed
	pid = min(MAX_SPEED_PID_BOOST, pid);
	pid = max(0, pid);

	return pid;
}

void PrintDebugInfo();
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

void *run(void *n) {
	volatile int percent;
	HRP_T hrp;
	short ifh, ifr, ifp;

	BNO055_Imu Imu;
	Imu.start();

	usleep(500000); //seems like it needs some time after initial start up

	/* Calibrate for initial IMU orientation */
	hrp = Imu.get_hrp();
	ifh = hrp.h;
	ifr = hrp.r;
	ifp = hrp.p;

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
		usleep(IMU_SLEEP_US); //wait for next IMU reading
		hrp = Imu.get_hrp(); //get IMU reading and update orientation
		Orientations[ORIENTATION_HEADING] = hrp.h - ifh;
		Orientations[ORIENTATION_ROLL] = hrp.r - ifr;
		Orientations[ORIENTATION_PITCH] = hrp.p - ifp;

		DebugInfo.imu_h = Orientations[ORIENTATION_HEADING];
		DebugInfo.imu_r = Orientations[ORIENTATION_ROLL];
		DebugInfo.imu_p = Orientations[ORIENTATION_PITCH];

		//stabilize
		percent = percent_g;
		MotorSpeed[FRONT] = percent + Pid(FRONT);
		MotorSpeed[BACK] = percent + Pid(BACK);
		MotorSpeed[RIGHT] = percent + Pid(RIGHT);
		MotorSpeed[LEFT] = percent + Pid(LEFT);

		//write motor value
		p0.set_12dc_percent(MotorSpeed[FRONT]);
		p1.set_12dc_percent(MotorSpeed[RIGHT]);
		p2.set_12dc_percent(MotorSpeed[BACK]);
		p3.set_12dc_percent(MotorSpeed[LEFT]);

		DebugInfo.Motor[FRONT].motorspeed = MotorSpeed[FRONT];
		DebugInfo.Motor[RIGHT].motorspeed = MotorSpeed[RIGHT];
		DebugInfo.Motor[BACK].motorspeed = MotorSpeed[BACK];
		DebugInfo.Motor[LEFT].motorspeed = MotorSpeed[LEFT];
		DebugInfo.setspeed = percent_g;

		//PrintDebugInfo();
		if (percent > 5)
			print_2_csv();
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
void ctrlc_handler(int s);
int main(int c, char *argv[]) {
	struct sigaction sigIntHandler;
	pthread_t input_thread;
	pthread_t run_thread;

	setup();

	sigIntHandler.sa_handler = ctrlc_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	//create all threads
	pthread_create(&input_thread, NULL, monitor_input, NULL);
	pthread_create(&run_thread, NULL, run, NULL);
	
	//wait for threads to finish
	pthread_join(run_thread, NULL);
	pthread_join(input_thread, NULL);

	cout << "Exiting..." << endl;
}

void PrintDebugInfo() {
	static int cnt = 100;

	if (--cnt == 0) {
		cnt = 100;
		printf("------\n");
		//printf("FRONT: %f, RIGHT: %f, BACK: %f, LEFT: %f\n", MotorSpeed[FRONT],
		//MotorSpeed[RIGHT], MotorSpeed[BACK], MotorSpeed[LEFT]);
		printf("ERROR:\n"
			   "  FRONT: %04hi, RIGHT: %04hi, BACK: %04hi, LEFT: %04hi\n", DebugInfo.Motor[0].error,
		DebugInfo.Motor[1].error, DebugInfo.Motor[2].error, DebugInfo.Motor[3].error);

		printf("PID:\n"
			   "  FRONT: %04hi, RIGHT: %04hi, BACK: %04hi, LEFT: %04hi\n", DebugInfo.Motor[0].pid,
		DebugInfo.Motor[1].pid, DebugInfo.Motor[2].pid, DebugInfo.Motor[3].pid);

		printf("MOTORSPEED:\n"
			   "  FRONT: %04hi, RIGHT: %04hi, BACK: %04hi, LEFT: %04hi\n", DebugInfo.Motor[0].motorspeed,
		DebugInfo.Motor[1].motorspeed, DebugInfo.Motor[2].motorspeed, DebugInfo.Motor[3].motorspeed);
		//printf("h: %hi, r: %hi, p: %hi\n", DebugInfo.imu_h, DebugInfo.imu_r, DebugInfo.imu_p);
	}
}

void ctrlc_handler(int s) {
	running_g = 0;
	usleep(20000);
	exit(0);
}

/* struct MotorPidDebug {
	short pterm, iterm, dterm;
	short pid;
	short error;
	short motorspeed;
};

struct {
	short setspeed;
	short imu_h, imu_r, imu_p;
	struct MotorPidDebug Motor[4];
} DebugInfo; */

void setup() {
	fdebug.open(DEBUG_FILENAME, ios::out);
	fdebug << "cnt,setspeed,heading,roll,pitch,"
			"front_p,front_i,front_d,front_pid,front_err,front_speed,"
			"right_p,right_i,right_d,right_pid,right_err,right_speed,"
			"back_p,back_i,back_d,back_pid,back_err,back_speed,"
			"left_p,left_i,left_d,left_pid,left_err,left_speed,\n";
}

void print_2_csv() {
	static int cnt = 0;

	fdebug << cnt++ << ",";
	fdebug << DebugInfo.setspeed << ",";
	fdebug << DebugInfo.imu_h << ",";
	fdebug << DebugInfo.imu_r << ",";
	fdebug << DebugInfo.imu_p << ",";

	fdebug << DebugInfo.Motor[FRONT].pterm << ",";
	fdebug << DebugInfo.Motor[FRONT].iterm << ",";
	fdebug << DebugInfo.Motor[FRONT].dterm << ",";
	fdebug << DebugInfo.Motor[FRONT].pid << ",";
	fdebug << DebugInfo.Motor[FRONT].error << ",";
	fdebug << DebugInfo.Motor[FRONT].motorspeed << ",";

	fdebug << DebugInfo.Motor[RIGHT].pterm << ",";
	fdebug << DebugInfo.Motor[RIGHT].iterm << ",";
	fdebug << DebugInfo.Motor[RIGHT].dterm << ",";
	fdebug << DebugInfo.Motor[RIGHT].pid << ",";
	fdebug << DebugInfo.Motor[RIGHT].error << ",";
	fdebug << DebugInfo.Motor[RIGHT].motorspeed << ",";

	fdebug << DebugInfo.Motor[BACK].pterm << ",";
	fdebug << DebugInfo.Motor[BACK].iterm << ",";
	fdebug << DebugInfo.Motor[BACK].dterm << ",";
	fdebug << DebugInfo.Motor[BACK].pid << ",";
	fdebug << DebugInfo.Motor[BACK].error << ",";
	fdebug << DebugInfo.Motor[BACK].motorspeed << ",";

	fdebug << DebugInfo.Motor[LEFT].pterm << ",";
	fdebug << DebugInfo.Motor[LEFT].iterm << ",";
	fdebug << DebugInfo.Motor[LEFT].dterm << ",";
	fdebug << DebugInfo.Motor[LEFT].pid << ",";
	fdebug << DebugInfo.Motor[LEFT].error << ",";
	fdebug << DebugInfo.Motor[LEFT].motorspeed << ",";

	fdebug << endl;
}