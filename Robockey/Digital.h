void initDigital();
void setEnabled(bool enabled);
//Positive is forward. 0 is off and 1600 is 100% duty cycle
void setMotors(int8_t right, int8_t left);
void startKick(uint16_t duration);
void updateKick();
bool switchesPressed();

enum LEDColor{
	OFF=0, RED=1 ,BLUE=2, PURPLE=3
};

void setLED(enum LEDColor color);