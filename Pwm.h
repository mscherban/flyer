#include <iostream>
#include <fstream>

using namespace std;

#define PWM_PATH	"/sys/class/pwm/"

class Pwm {
	public:
	string name;
	
	Pwm(string Name, string Pwm_Dev);
	~Pwm();
	void enable();
	void disable();
	void set_period(int ns);
	void set_duty_cycle(int percent);
	bool ok();
	
	private:
	int period_ns, duty_cycle_p;
	string dev;
	fstream fenable, fduty_cycle, fperiod;
};