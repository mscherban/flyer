/*
	This is an object file for the BNO055 Fusion Breakout Board by Adafruit.
	https://learn.adafruit.com/adafruit-bno055-absolute-orientation-sensor
	
	Communicates over bbb i2c2: SCL -- P9.19 | SDA -- P9.20
*/

#ifndef BNO055_IMU_H
#define BNO055_IMU_H

#define BBB_I2C2_FILENAME	"/dev/i2c-2"
#define BNO055_CHIP_ID		0xA0

#define BNO055_OPR_MODE		0x3D
#define BNO055_ADR			0x28 /* breakout board pulls down COM3,
									setting this to 0x28 */

#define OPR_MODE_IMU		0x08
#define OPR_MODE_CONFIG		0x00

/* 
	All axis are 2 bytes starting at:
	Heading - 0x1A
	Roll    - 0x1C
	Pitch   - 0x1E
 */
#define HEADING_ADR	0x1A
#define ROLL_ADR	0x1C
#define PITCH_ADR	0x1E

typedef struct {
	short h, r, p;
	float fh, fr, fp;
} HRP_T;

class BNO055_Imu {
	public:
	BNO055_Imu();
	~BNO055_Imu();
	void config();
	void start();
	void stop();
	HRP_T get_hrp();
	
	private:
	int fd;
};

#endif