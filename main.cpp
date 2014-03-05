#include "xhw_types.h"
#include "xhw_memmap.h"
#include "xspi.h"
#include "xhw_spi.h"
#include "xhw_sysctl.h"
#include "xsysctl.h"
#include "xgpio.h"
#include "xcore.h"
#include "xpwm.h"
#include "xwdt.h"
/*#include "xhw_uart.h"
#include "xuart.h"*/

#include <stdint.h>

/*void printstring(char * data){
	while (*data != '\0') {
		UARTCharPut(UART0_BASE,*data++);
	}
}*/

#ifdef __cplusplus
extern "C" {
#endif

static unsigned long milliSec = 0;

int output = 0;

void SysTickIntHandler() {
	milliSec++;
}

uint32_t millis() {
	return milliSec;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#ifdef __cplusplus
}
#endif

#include "rf24/RF24.h"
#include "report.h"

static unsigned long ulClockMS = 0;
static unsigned long timeoutcounter = 0;

void rf24_init(RF24& radio) {
	const uint64_t pipes[2] = { 0xF0F0F0F0BELL, 0xF0F0F0F0EFLL };
	radio.begin();
	radio.setChannel(100);
	radio.setRetries(15, 15);
	radio.setPayloadSize(sizeof(report_t));
	radio.setDataRate(RF24_250KBPS);
	radio.openWritingPipe(pipes[1]);
	radio.openReadingPipe(1, pipes[0]);
	radio.startListening();
	radio.printDetails();
}

void pwmInit(){
	xSysCtlPeripheralClockSourceSet(xSYSCTL_PWMA_HCLK, 1);
	xSysCtlPeripheralEnable(SYSCTL_PERIPH_PWMA);
	xSysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	xSPinTypePWM(TIM0CH0, PB11);
	xSPinTypePWM(TIM0CH1, PB10);

	xPWMInitConfigure(xPWMA_BASE, xPWM_CHANNEL0, xPWM_TOGGLE_MODE);
	xPWMInitConfigure(xPWMA_BASE, xPWM_CHANNEL1, xPWM_TOGGLE_MODE);
	xPWMFrequencySet(xPWMA_BASE, xPWM_CHANNEL0, 50);
	xPWMDutySetPrec(xPWMA_BASE, xPWM_CHANNEL0, 750);
	xPWMOutputEnable(xPWMA_BASE, xPWM_CHANNEL0);
	xPWMStart(xPWMA_BASE, xPWM_CHANNEL1);
	xPWMFrequencySet(xPWMA_BASE, xPWM_CHANNEL1, 50);
	xPWMDutySetPrec(xPWMA_BASE, xPWM_CHANNEL1, 750);
	xPWMOutputEnable(xPWMA_BASE, xPWM_CHANNEL1);
	xPWMStart(xPWMA_BASE, xPWM_CHANNEL1);
}

void stopall(){
	xPWMDutySetPrec(xPWMA_BASE, xPWM_CHANNEL0,750);
	xPWMDutySetPrec(xPWMA_BASE, xPWM_CHANNEL1,750);
}

#define STEERRANGE 200
#define STEERMIDDLE 750
#define SYSTICKS_PER_SECOND     1000
#define TIMEOUT 240

static unsigned middle=STEERMIDDLE;


void sendStatus(report_t& gamepad_report, RF24& radio) {
		// Delay just a little bit to let the other unit
		// make the transition to receiver
		xSysCtlDelay(ulClockMS * 10);

		radio.stopListening();
		uint8_t response = 0;
		radio.write(&response, sizeof(uint8_t));
		radio.startListening();
}


#define DELTATIME 100
void calibrate(RF24& radio) {
	bool finished = false;

	xPWMDutySetPrec(xPWMA_BASE, xPWM_CHANNEL1, middle);
	
	unsigned long timestartright;
	unsigned long timestartleft;

	bool rightdown = false;
	bool leftdown = false;
	

	do{

		// if there is data ready
		if (radio.available()) {

			report_t gamepad_report;
			bool done = false;

			while (!done) {

				done = radio.read(&gamepad_report, sizeof(report_t));

				if (RIGHT_PRESSED(gamepad_report)){
					if(rightdown){
						if ((millis()-timestartright)>DELTATIME){
							middle++;
						}
					}else{
						rightdown=true;
						timestartright=millis();
					}
				}else{
					rightdown=false;
				}


				if (LEFT_PRESSED(gamepad_report)){
					if(leftdown){
						if ((millis()-timestartleft)>DELTATIME){
							middle--;
						}
					}else{
						leftdown=true;
						timestartleft=millis();
					}
				}else{
					leftdown=false;
				}

				xPWMDutySetPrec(xPWMA_BASE, xPWM_CHANNEL1, middle);

				//finish
				if (START_PRESSED(gamepad_report)){
					finished=true;
				}

				//send status if needed
				if (gamepad_report.reportid == 1) {
					sendStatus(gamepad_report, radio);
				}
			}

		}
	}while(!finished);
}

int main() {

	//
	// Disable the watchdog
	//
	WDTimerDisable();

	xSysCtlClockSet32KhzFLLExt(); //48mhz from 32768khz external clock
	xSysCtlDelay(100000);


	xSysTickPeriodSet(xSysCtlClockGet()/SYSTICKS_PER_SECOND); //1ms
	xSysTickIntEnable();
	xSysTickEnable(); // End of SysTick init
	ulClockMS = xSysCtlClockGet() / (3 * 1000);

	//enable uart
	/*xSysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	xSysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	xSPinTypeUART(UART0RX, PB2);
    xSPinTypeUART(UART0TX, PB1);
    SysCtlPeripheralClockSourceSet(SYSCTL_PERIPH_UART0_S_MCGPLLCLK_2);
    UARTDisable(UART0_BASE, UART_TX | UART_RX);
    UARTConfigSet(UART0_BASE, 115200, UART_CONFIG_SAMPLE_RATE_15 | UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_1);
    UARTEnable(UART0_BASE, UART_TX | UART_RX);*/

	pwmInit();

	// Setup and configure rf radio
	RF24 radio = RF24();
	rf24_init(radio);

	while (1) {

		// if there is data ready
		if (radio.available()) {

			report_t gamepad_report;
			bool done = false;

			while (!done) {

				done = radio.read(&gamepad_report, sizeof(report_t));
				int x = (int8_t)(gamepad_report.x);
				int y = (int8_t)(gamepad_report.y);
				int rx = (int8_t)(gamepad_report.rx);
				int ry = (int8_t)(gamepad_report.ry);

				if (X_PRESSED(gamepad_report) && Y_PRESSED(gamepad_report)){
					//printstring("calibrating\n");
					calibrate(radio);
					break;
				}

				//steering (servo)
				unsigned int pos = middle;
				if (x > 0) {
					pos = map(x, 1, 127, middle + 1, middle + STEERRANGE);
				} else if (x < 0) {
					pos = map(-x, 1, 127, middle - 1, middle - STEERRANGE);
				}
				xPWMDutySetPrec(xPWMA_BASE, xPWM_CHANNEL1, pos);

				//speed (esc)
				unsigned int speed = 750;
				if (ry > 0) {
					speed = map(ry, 1, 127, 749, 250);
				} else if (ry < 0) {
					speed = map(-ry, 1, 127, 751, 1250);
				}
				xPWMDutySetPrec(xPWMA_BASE, xPWM_CHANNEL0, speed);

				//send status if needed
				if (gamepad_report.reportid == 1) {
					sendStatus(gamepad_report, radio);
				}
			}

			timeoutcounter = millis();

		} else {
			if ((millis() - timeoutcounter) > TIMEOUT) {
				stopall();
				xSysCtlDelay(ulClockMS * 10);
			}
		}



	}

	return 0;

}

