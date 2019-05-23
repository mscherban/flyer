#!/bin/bash

#list of pwm pins:
#pwm-1:0 -- P9.22
#pwm-1:1 -- P9.21
#pwm-4:0 -- P9.14
#pwm-4:1 -- P9.16

#list of i2c pins:
#I2C2 -- SCL -- P9.19
#I2C2 -- SDA -- P9.20

if $(config-pin -q P9.22 | grep -q pwm); then
	echo P9.22 already configured as pwm.
else
	echo Setting P9.22 to pwm.
	config-pin P9.22 pwm #pwm-1:0
fi

if $(config-pin -q P9.21 | grep -q pwm); then
	echo P9.21 already configured as pwm.
else
	echo Setting P9.21 to pwm.
	config-pin P9.21 pwm #pwm-1:1
fi

if $(config-pin -q P9.14 | grep -q pwm); then
	echo P9.14 already configured as pwm.
else
	echo Setting P9.14 to pwm.
	config-pin P9.14 pwm #pwm-4:0
fi

if $(config-pin -q P9.16 | grep -q pwm); then
	echo P9.16 already configured as pwm.
else
	echo Setting P9.16 to pwm.
	config-pin P9.16 pwm #pwm-4:1
fi

if $(config-pin -q P9.19 | grep -q i2c); then
	echo P9.19 already configured as i2c.
else
	echo Setting P9.19 to i2c.
	config-pin P9.19 i2c #
fi

if $(config-pin -q P9.20 | grep -q i2c); then
	echo P9.20 already configured as i2c.
else
	echo Setting P9.20 to i2c.
	config-pin P9.20 i2c #
fi

./main.out
./killpwms.out