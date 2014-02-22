/*
 * spi.c
 *
 *  Created on: Apr 21, 2013
 *      Author: bgouveia
 */

#include "xhw_types.h"
#include "xhw_memmap.h"
#include "xspi.h"
#include "xhw_spi.h"
#include "xhw_sysctl.h"
#include "xsysctl.h"
#include "xgpio.h"
#include "xcore.h"

#include "spi.h"

//CE pb12
//CS pa19

static unsigned long ulClockMS=0;

void spi_init(unsigned long bitrate,unsigned long datawidth){

    //
    // Enable Peripheral SPI0
    //
    xSysCtlPeripheralEnable(SYSCTL_PERIPH_SPI0);
    xSysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    xSysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    xSPIConfigSet(xSPI0_BASE, 5000000, (xSPI_MOTO_FORMAT_MODE_0 | xSPI_MODE_MASTER | xSPI_MSB_FIRST));

    //
    // Configure Some GPIO pins as SPI Mode
    //
    xSPinTypeSPI(SPI0CLK,  PB17);
    xSPinTypeSPI(SPI0MISO, PB15);
    xSPinTypeSPI(SPI0MOSI, PB16);
    xGPIOSPinTypeGPIOOutput(PB12); //CE
    xGPIOSPinTypeGPIOOutput(PA19);//CS

    //SPISSSet(xSPI0_BASE,SPI_SS_OUTPUT);

    xSPIEnable(xSPI0_BASE);

    ulClockMS = xSysCtlClockGet() / (3 * 1000);
}

void spi_cs_low()
{
	xGPIOPinWrite(xGPIO_PORTA_BASE, xGPIO_PIN_19, 0);
}

void spi_cs_high()
{
	xGPIOPinWrite(xGPIO_PORTA_BASE, xGPIO_PIN_19, 1);
}

void spi_ce_low()
{
	xGPIOPinWrite(xGPIO_PORTB_BASE, xGPIO_PIN_12, 0);
}

void spi_ce_high()
{
	xGPIOPinWrite(xGPIO_PORTB_BASE, xGPIO_PIN_12, 1);
}


uint8_t spi_transferByte(uint8_t data){

	unsigned long rxdata;
	rxdata = xSPISingleDataReadWrite(xSPI0_BASE, data);
	return (rxdata&0xFF);
}

void delay(unsigned long msec){
	xSysCtlDelay(ulClockMS*msec);
}

void delayMicroseconds(unsigned long usec){
	xSysCtlDelay((ulClockMS/1000)*usec);
}
