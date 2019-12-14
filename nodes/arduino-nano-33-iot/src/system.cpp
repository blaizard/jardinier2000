#include "system.h"

#include <Arduino.h>

uint32_t node::system::timestamp = 0;

void TC3_Handler()
{
	++node::system::timestamp;
	TC3->COUNT16.INTFLAG.bit.MC0 = 1;
}

void WDT_Handler()
{
	WDT->CTRL.bit.ENABLE = 0;        // Disable watchdog
	while (WDT->STATUS.bit.SYNCBUSY); // Sync CTRL write
    WDT->INTFLAG.bit.EW  = 1;        // Clear interrupt flag
}

namespace
{
	void gclkWaitBusy()
	{
		while (GCLK->STATUS.bit.SYNCBUSY);
	}

	/**
	 * Configure generic clock
	 * 
	 * \note
	 * Generic clock generator 0 8 division factor bits - DIV[7:0]
	 * Generic clock generator 1 16 division factor bits - DIV[15:0]
	 * Generic clock generators 2 5 division factor bits - DIV[4:0]
	 * Generic clock generators 3 - 8 8 division factor bits - DIV[7:0]
	 */
	template <uint32_t genClk, uint32_t src, uint32_t dst, uint32_t gclkDiv>
	void gclkSetup()
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

		GCLK->GENDIV.reg = GCLK_GENDIV_ID(genClk) | GCLK_GENDIV_DIV(gclkDiv);
		gclkWaitBusy();

		// Use the OSCULP32K for GCLK2, divide it, 50/50 duty cycle, 
		GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(genClk) | GCLK_GENCTRL_DIVSEL | GCLK_GENCTRL_GENEN | src;   
		gclkWaitBusy();

		// Feed GCLK2 to WDT (Watchdog Timer)
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_GEN(genClk) | GCLK_CLKCTRL_CLKEN | dst;
		gclkWaitBusy();
	}

	void timerStop()
	{
		// Disbale the IRQ
		NVIC_DisableIRQ(TC3_IRQn);
		NVIC_ClearPendingIRQ(TC3_IRQn);

		TC3->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
		while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
		while (TC3->COUNT16.CTRLA.bit.SWRST);
	}

	template <uint32_t genClk>
	void timerSetup(const uint32_t sampleRate)
	{
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_GEN(genClk) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_TCC2_TC3;
		while (GCLK->STATUS.bit.SYNCBUSY);

		// Reset the timer
		timerStop();

		// Set Timer counter Mode to 16 bits
		TC3->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
		// Set TC3 mode as match frequency
		TC3->COUNT16.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
		//set prescaler and enable TC3
		TC3->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1 | TC_CTRLA_ENABLE;
		//set TC3 timer counter based off of the system clock and the user defined sample rate or waveform
		TC3->COUNT16.CC[0].reg = (uint16_t) (SystemCoreClock / sampleRate - 1);
		while (TC3->COUNT16.STATUS.bit.SYNCBUSY);

		 // Configure interrupt request
		NVIC_DisableIRQ(TC3_IRQn);
		NVIC_ClearPendingIRQ(TC3_IRQn);
		NVIC_SetPriority(TC3_IRQn, 0);
		NVIC_EnableIRQ(TC3_IRQn);

		 // Enable the TC3 interrupt request
		TC3->COUNT16.INTENSET.bit.MC0 = 1;
		while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
	}

	void watchdogClear()
	{
		WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
		while (WDT->STATUS.bit.SYNCBUSY);
	}

	void watchdogStop()
	{
		NVIC_DisableIRQ(WDT_IRQn);
		NVIC_ClearPendingIRQ(WDT_IRQn);

		WDT->CTRL.reg = 0;
		while (WDT->STATUS.bit.SYNCBUSY);
	}

	template <uint32_t genClk, uint32_t src, uint32_t gclkDiv>
	void watchdogSetup(const uint32_t timeoutPeriod)
	{
		// Disable watchdog for config
		watchdogStop();
	
		::gclkSetup<genClk, src, GCLK_CLKCTRL_ID_WDT, gclkDiv>();

		// Set period for chip reset
		WDT->CONFIG.bit.WINDOW = 0;

		// For interrupt
		// Enable early warning interrupt
		WDT->INTENSET.bit.EW = 1;
		// Set max period
        WDT->CONFIG.bit.PER = 0xB;
        WDT->CONFIG.bit.WINDOW = timeoutPeriod;
		// Disable window mode
        WDT->CTRL.bit.WEN = 1;

		//WDT->CONFIG.bit.PER = timeoutPeriod;
		// Disable early warning interrupt
        //WDT->INTENCLR.bit.EW = 1;
		// Disable window mode
        //WDT->CTRL.bit.WEN = 0;
        while (WDT->STATUS.bit.SYNCBUSY);

		// Clear the watchdog timer
		watchdogClear();

		// Enable WDT early-warning interrupt
		NVIC_SetPriority(WDT_IRQn, 0); // Top priority
		NVIC_EnableIRQ(WDT_IRQn);

		WDT->CTRL.bit.ENABLE = 1; 
		while (WDT->STATUS.bit.SYNCBUSY);

		// Nb gclk per second, where 0 <= gclkDiv <= 16
		// nbGclkPerS = (32786 / (2 << gclkDiv))
		// TargetTimeS * nbGclkPerS = timeoutPeriod
		// TargetTimeS = timeoutPeriod / (32786 / (2 << gclkDiv))
		// 16000 / (32786 / (2 ^ (9 + 1)))
	}
}

void node::system::start()
{
	// Switch on builtin LED
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	// GCLK4 = 8MHz
	::gclkSetup<4, GCLK_GENCTRL_SRC_OSC8M, 0, 0>();

	// Setup the timer for the timestamp at 1KHz
	::timerSetup<0>(1000);

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

	::timerStop();

	// Notify that the application is in sleep
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
}

void node::system::sleep()
{
	stop();

	// A device enters STANDBY when a Wait For Interrupt (WFI) instruction is executed while SCR.DEEPSLEEP is set to '1'.
	// An interrupt or the Watch Dog Timer (WDT) will cause the device to wake-up and resume operation.
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

	// Data sync to ensure outgoing memory accesses complete
	__DSB();
	// Wait for interrupt (places device in sleep mode)
	__WFI();

	start();
}

void node::system::sleepFor16s()
{
	log::info<Topic>("Sleeping for 16s...");
	::watchdogSetup<3, GCLK_GENCTRL_SRC_OSCULP32K, 4>(WDT_CONFIG_PER_16K_Val);
	sleep();
	timestamp += 16 * 1000;
}

void node::system::sleepFor1min()
{
	log::info<Topic>("Sleeping for 1min...");
	::watchdogSetup<3, GCLK_GENCTRL_SRC_OSCULP32K, 6>(WDT_CONFIG_PER_16K_Val);
	sleep();
	timestamp += 60 * 1000;
}

void node::system::sleepFor8min()
{
	log::info<Topic>("Sleeping for 8min...");
	::watchdogSetup<1, GCLK_GENCTRL_SRC_OSCULP32K, 9>(WDT_CONFIG_PER_16K_Val);
	sleep();
	timestamp += 8 * 60 * 1000;
}

void node::system::sleepFor16min()
{
	log::info<Topic>("Sleeping for 16min...");
	::watchdogSetup<1, GCLK_GENCTRL_SRC_OSCULP32K, 10>(WDT_CONFIG_PER_16K_Val);
	sleep();
	timestamp += 16 * 60 * 1000;
}

void node::system::sleepFor32min()
{
	log::info<Topic>("Sleeping for 32min...");
	::watchdogSetup<1, GCLK_GENCTRL_SRC_OSCULP32K, 11>(WDT_CONFIG_PER_16K_Val);
	sleep();
	timestamp += 32 * 60 * 1000;
}

void node::system::restart()
{
	NVIC_SystemReset();
	while (true);
}
