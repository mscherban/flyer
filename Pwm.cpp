#include <iostream>
#include "Pwm.h"

PwmInstance Pwms[4] = {
	[0] = {.name = "One", .pwm_dev = "pwm-1:0"},
	[1] = {.name = "Two", .pwm_dev = "pwm-1:1"},
	[2] = {.name = "Three", .pwm_dev = "pwm-4:0"},
	[3] = {.name = "Four", .pwm_dev = "pwm-4:1"},
};

Pwm::Pwm(string Name, string Pwm_Dev) {
	string o = "PWM " + Name + "(";
	o += Pwm_Dev + ") Initializing";
	cout << o << endl;

	this->name = Name;
	this->dev = PWM_PATH + Pwm_Dev;
	
	this->fenable.open(this->dev + "/enable", ios::out);
	if (!this->fenable.is_open()) {
		cout << "File Open Error" << endl;
	}
	
	this->fduty_cycle.open(this->dev + "/duty_cycle", ios::out);
	if (!this->fduty_cycle.is_open()) {
		cout << "File Open Error" << endl;
	}
	
	this->fperiod.open(this->dev + "/period", ios::out);
	if (!this->fperiod.is_open()) {
		cout << "File Open Error" << endl;
	}
}

Pwm::~Pwm() {
	this->disable();
	this->fenable.close();
	this->fduty_cycle.close();
	this->fperiod.close();
	cout << "Closing " << this->name << endl;
}

bool Pwm::ok() {
	return this->fenable.is_open() &&
			this->fduty_cycle.is_open() &&
			this->fperiod.is_open();
}

void Pwm::enable() {
	this->fenable << "1" << endl;
}

void Pwm::disable() {
	this->fenable << "0" << endl;
}

void Pwm::set_period(int ns) {
	this->period_ns = ns;
	this->fperiod << to_string(ns) << endl;
}

void Pwm::set_duty_cycle(int percent) {
	int set = (this->period_ns * percent) / 100;
	if (set >= 0 && set <= this->period_ns) {
		this->duty_cycle_p = set;
		this->fduty_cycle << to_string(set) << endl;
	}
}

/* This fucntion sets the duty cycle to be between 1-2ms */
void Pwm::set_12dc_percent(float percent) {
	int dc;
	if (percent >= 0.0 && percent <= 100.0) {
		dc = 1000000; //1 milisecond minimum;
		dc += (int)((percent/100)*1000000);
		this->fduty_cycle << to_string(dc) << endl;
	}
}