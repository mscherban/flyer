#!/bin/bash

#list of pwm pins:
#pwm-1:0
#pwm-1:1
#pwm-4:0
#pwm-4:1

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
	config-pin P9.21 pwm #pwm-1:0
fi

if $(config-pin -q P9.14 | grep -q pwm); then
	echo P9.14 already configured as pwm.
else
	echo Setting P9.14 to pwm.
	config-pin P9.14 pwm #pwm-1:0
fi

if $(config-pin -q P9.16 | grep -q pwm); then
	echo P9.16 already configured as pwm.
else
	echo Setting P9.16 to pwm.
	config-pin P9.16 pwm #pwm-1:0
fi

./main.out
