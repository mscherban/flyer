#ifndef PWM_H
#define PWM_H

#include <fstream>

using namespace std;

#define PWM_PATH	"/sys/class/pwm/"

/* Motor locations relative to the IMU */
enum Motor_Locs {
	FRONT = 0,
	RIGHT = 1,
	BACK = 2,
	LEFT = 3,
};

typedef struct {
	string name;
	string pwm_dev;
} PwmInstance;

class Pwm {
	public:
	string name;
	
	Pwm(string Name, string Pwm_Dev);
	~Pwm();
	void enable();
	void disable();
	void set_period(int ns);
	void set_duty_cycle(int percent);
	void set_12dc_percent(int percent);
	bool ok();
	
	private:
	int period_ns, duty_cycle_p;
	string dev;
	fstream fenable, fduty_cycle, fperiod;
};

#endif /* PWM_H */