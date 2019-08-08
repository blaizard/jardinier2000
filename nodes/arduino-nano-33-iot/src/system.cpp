#include "system.h"
#include "persistence.h"

#include <Arduino.h>

namespace
{
	template <uint8_t gclkDiv, uint8_t timeoutPeriod>
	void watchdogSetup()
	{
		// Divide GLCK2 by 2 ^ (gclkDiv + 1)
		REG_GCLK_GENDIV = GCLK_GENDIV_DIV(gclkDiv) | GCLK_GENDIV_ID(2);
		while (GCLK->STATUS.bit.SYNCBUSY);

		// Use the OSCULP32K for GCLK2, divide it, 50/50 duty cycle, 
		REG_GCLK_GENCTRL = GCLK_GENCTRL_DIVSEL | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_ID(2);   
		while (GCLK->STATUS.bit.SYNCBUSY);

		// Feed GCLK2 to WDT (Watchdog Timer)
		REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2 | GCLK_CLKCTRL_ID_WDT;
		while (GCLK->STATUS.bit.SYNCBUSY);

		REG_WDT_CONFIG = timeoutPeriod;
		while(WDT->STATUS.bit.SYNCBUSY);

		REG_WDT_CTRL = WDT_CTRL_ENABLE;
		while(WDT->STATUS.bit.SYNCBUSY);

		// Clear the watchdog timer
		REG_WDT_CLEAR = WDT_CLEAR_CLEAR_KEY;
		while(WDT->STATUS.bit.SYNCBUSY);

		// Nb gclk per second, where 0 <= gclkDiv <= 16
		// nbGclkPerS = (32786 / (2 << gclkDiv))
		// TargetTimeS * nbGclkPerS = timeoutPeriod
		// TargetTimeS = timeoutPeriod / (32786 / (2 << gclkDiv))
		// 16000 / (32786 / (2 ^ (8 + 1)))
	}
}

void node::system::rebootAfter16s()
{
	::watchdogSetup<4, WDT_CONFIG_PER_16K_Val>();
	sleep();
}

void node::system::rebootAfter1min()
{
	::watchdogSetup<6, WDT_CONFIG_PER_16K_Val>();
	sleep();
}

void node::system::rebootAfter8min()
{
	::watchdogSetup<9, WDT_CONFIG_PER_16K_Val>();
	sleep();
}

void node::system::reboot()
{
	NVIC_SystemReset();
	while (true);
}

void node::system::sleep()
{
	// Save the persisted content
	persistence::Data.write();

	// Disconnect the USB device
	USBDevice.detach();
	USBDevice.end();
	USBDevice.standby();
	//USB->DEVICE.CTRLA.bit.SWRST = 1;
	//while (USB->DEVICE.SYNCBUSY.bit.SWRST | USB->DEVICE.CTRLA.bit.SWRST == 1);

	// Notify that the application is in sleep
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	// A device enters STANDBY when a Wait For Interrupt (WFI) instruction is executed while SCR.DEEPSLEEP is set to '1'.
	// An interrupt or the Watch Dog Timer (WDT) will cause the device to wake-up and resume operation.
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	__DSB();
	__WFI();
	reboot();
}
