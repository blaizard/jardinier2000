#include "system.h"

#include <Arduino.h>

namespace
{
	void watchdogClear()
	{
		WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
		while (WDT->STATUS.bit.SYNCBUSY);
	}

	/**
	 * Generic clock generator 0 8 division factor bits - DIV[7:0]
	 * Generic clock generator 1 16 division factor bits - DIV[15:0]
	 * Generic clock generators 2 5 division factor bits - DIV[4:0]
	 * Generic clock generators 3 - 8 8 division factor bits - DIV[7:0]
	 */
	template <uint32_t gclkDiv, uint32_t timeoutPeriod, uint32_t genClk = 3>
	void watchdogSetup()
	{
		static_assert(genClk <= 8);

		static_assert(genClk != 0 || gclkDiv <= 8);
		static_assert(genClk != 1 || gclkDiv <= 16);
		static_assert(genClk != 2 || gclkDiv <= 5);
		static_assert(genClk != 3 || gclkDiv <= 8);
		static_assert(genClk != 4 || gclkDiv <= 8);
		static_assert(genClk != 5 || gclkDiv <= 8);
		static_assert(genClk != 6 || gclkDiv <= 8);
		static_assert(genClk != 7 || gclkDiv <= 8);
		static_assert(genClk != 8 || gclkDiv <= 8);

		// Disable watchdog for config
		WDT->CTRL.reg = 0;
		while (WDT->STATUS.bit.SYNCBUSY);
	
		GCLK->GENDIV.reg = GCLK_GENDIV_ID(genClk) | GCLK_GENDIV_DIV(gclkDiv);
		while (GCLK->STATUS.bit.SYNCBUSY);

		// Use the OSCULP32K for GCLK2, divide it, 50/50 duty cycle, 
		GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(genClk) | GCLK_GENCTRL_DIVSEL | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K;   
		while (GCLK->STATUS.bit.SYNCBUSY);

		// Feed GCLK2 to WDT (Watchdog Timer)
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_GEN(genClk) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_WDT;
		while (GCLK->STATUS.bit.SYNCBUSY);

		// Set period for chip reset
		WDT->CONFIG.bit.WINDOW = 0;
		WDT->CONFIG.bit.PER = timeoutPeriod;
		// Disable early warning interrupt
        WDT->INTENCLR.bit.EW = 1;
		// Disable window mode
        WDT->CTRL.bit.WEN = 0;
        while (WDT->STATUS.bit.SYNCBUSY);

		// Clear the watchdog timer
		watchdogClear();

		WDT->CTRL.bit.ENABLE = 1; 
		while (WDT->STATUS.bit.SYNCBUSY);

		// Nb gclk per second, where 0 <= gclkDiv <= 16
		// nbGclkPerS = (32786 / (2 << gclkDiv))
		// TargetTimeS * nbGclkPerS = timeoutPeriod
		// TargetTimeS = timeoutPeriod / (32786 / (2 << gclkDiv))
		// 16000 / (32786 / (2 ^ (9 + 1)))
	}

	void sleepAndRestart()
	{
		// Stop the system
		node::system::stop();

		// A device enters STANDBY when a Wait For Interrupt (WFI) instruction is executed while SCR.DEEPSLEEP is set to '1'.
		// An interrupt or the Watch Dog Timer (WDT) will cause the device to wake-up and resume operation.
		SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

		// Data sync to ensure outgoing memory accesses complete
		__DSB();
		// Wait for interrupt (places device in sleep mode)
		__WFI();

		node::system::restart();
	}
}

void node::system::start()
{
	// Switch on builtin LED
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	// Initialize serial port
	Serial.begin(115200);
}

void node::system::stop()
{
	// Disconnect the USB device
	USBDevice.detach();
	USBDevice.end();
	USBDevice.standby();
	//USB->DEVICE.CTRLA.bit.SWRST = 1;
	//while (USB->DEVICE.SYNCBUSY.bit.SWRST | USB->DEVICE.CTRLA.bit.SWRST == 1);

	// Notify that the application is in sleep
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
}

void node::system::restartAfter16s()
{
	log::info<Topic>("Restarting in 1min...");
	::watchdogSetup<4, WDT_CONFIG_PER_16K_Val>();
	::sleepAndRestart();
}

void node::system::restartAfter1min()
{
	log::info<Topic>("Restarting in 1min...");
	::watchdogSetup<6, WDT_CONFIG_PER_16K_Val>();
	::sleepAndRestart();
}

void node::system::restartAfter8min()
{
	log::info<Topic>("Restarting in 8min...");
	::watchdogSetup<9, WDT_CONFIG_PER_16K_Val, 1>();
	::sleepAndRestart();
}

void node::system::restartAfter16min()
{
	log::info<Topic>("Restarting in 16min...");
	::watchdogSetup<10, WDT_CONFIG_PER_16K_Val, 1>();
	::sleepAndRestart();
}

void node::system::restartAfter32min()
{
	log::info<Topic>("Restarting in 32min...");
	::watchdogSetup<11, WDT_CONFIG_PER_16K_Val, 1>();
	::sleepAndRestart();
}

void node::system::restart()
{
	NVIC_SystemReset();
	while (true);
}
