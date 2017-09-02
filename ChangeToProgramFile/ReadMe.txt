The file kinetis.h in 
C:\Program Files (x86)\Arduino\hardware\teensy\avr\cores\teensy3
(or corresponding location on your operating system) should be replaced with 
The parameters:
	IRQ_PIT_CH0 =		68
	IRQ_PIT_CH1 =		69
	IRQ_PIT_CH2 =		70
	IRQ_PIT_CH3 =		71
Where missing for the Teensy LC. The numbers I put in for them were taken from another type of Teensy, so probably are not correct. If something isn’t working correctly, this may be something we can modify…
