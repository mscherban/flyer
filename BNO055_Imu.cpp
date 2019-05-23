/*
	This is an object file for the BNO055 Fusion Breakout Board by Adafruit.
	https://learn.adafruit.com/adafruit-bno055-absolute-orientation-sensor
*/

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "BNO055_Imu.h"

using namespace std;

BNO055_Imu::BNO055_Imu() {
	char buff = 0x00;

	if ((this->fd = open(BBB_I2C2_FILENAME, O_RDWR)) < 0) {
		cout << "Error opening i2c dev." << endl;
		return;
	}
	
	ioctl(this->fd, I2C_SLAVE, BNO055_ADR);
	write(this->fd, &buff, 1);
	read(this->fd, &buff, 1);
	
	if (buff == BNO055_CHIP_ID) {
		cout << "Connected to BNO055." << endl;
	}
}

BNO055_Imu::~BNO055_Imu() {
	if (this->fd != -1) {
		close(this->fd);
	}
}

void BNO055_Imu::config() {
	
}

/* setting the IMU to IMU mode starts the data output */
void BNO055_Imu::start() {
	char data[2] = { BNO055_OPR_MODE, OPR_MODE_IMU };
	write(this->fd, data, 2);
}

void BNO055_Imu::stop() {
	char data[2] = { BNO055_OPR_MODE, OPR_MODE_CONFIG };
	write(this->fd, data, 2);
}

HRP_T BNO055_Imu::get_hrp() {
	char adr = HEADING_ADR; 
	HRP_T hrp;
	
	/* set read adr to heading */
	write(this->fd, &adr, 1);
	/* read out 6 bytes, alignment matches HRP_T structure */
	read(this->fd, &hrp, 6);
	
	/* convert to float */
	hrp.fh = (float)hrp.h / 16;
	hrp.fr = (float)hrp.r / 16;
	hrp.fp = (float)hrp.p / 16;
	
	return hrp;
}