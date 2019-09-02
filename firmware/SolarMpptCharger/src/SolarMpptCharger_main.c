//=========================================================
// src/SolarMpptCharger_main.c: generated by Hardware Configurator
//
// 12V MPPT Solar Charger system for danjuliodesigns, LLC
// "MPPT SOLAR CHARGER" PCB designed to work with commonly
// available 36-cell 15-35W "12V" Solar Panels and 7Ah - 18 Ah
// 12V AGM sealed lead acid batteries and power 5V-based devices
// with an I2C interface and status signals.
//
// Copyright (c) 2018-2019 danjuliodesigns, LLC.  All rights reserved.
//
// Implements the following functionality:
//   1. MPPT Charge controller with 3 charge states:
//     a. BULK - high voltage, high current bulk charge
//     b. ABSORPTION - high voltage finishing charge
//     c. FLOAT - low voltage maintenance charge
//   2. MPPT Buck converter controller
//   3. Temperature compensation with external temperature
//      sensor and internal temperature sensor fallback
//   4. Load control for discharged and recharged battery
//      conditions (low-battery cutoff)
//   5. I2C configuration and monitoring interface
//   6. LED status indicator, Night detection and Alert
//      status outputs
//   7. Application enabled watchdog monitor to power-cycle the
//      external device if it does not regularly communicate
//      with the charger.
//
//
// Target: Silicon Labs EFM8SB10F8G (8K Flash, 512B RAM)
//
// PCB Assembly Compatibility: 35-00082-01 / 35-00082-02
//
// IO:
//   P0.0 - Unused (open-drain output)
//   P0.1 - IS ADC Input - Solar current (scaled by 1/2)
//   P0.2 - VB ADC Input - Battery voltage (scaled by 1/15)
//   P0.3 - IS ADC Input - Battery load current (scaled by 1/2)
//   P0.4 - Temp Input - 10 mV/C with 500 mV offset (0C)
//   P0.5 - VS ADC Input - Solar voltage (scaled by 1/15)
//   P0.6 - PWR_EN Digital Push/Pull output - Enable 5V output
//   P0.7 - NIGHT Digital Push/Pull output - Status
//          1: VS <= Night voltage threshold
//          0: VS > Night voltage threshold
//   P1.0 - ALERT_N Digital Push/Pull output - Active-low Status
//          1: Battery voltage good
//          0: Battery voltage low - power will switch off in 60s
//   P1.1 - I2C SDA
//   P1.2 - I2C SCL
//   P1.3 - MPPT Buck PWM Digital Push/Pull output
//   P1.4 - LED Indicator Digital Push/Pull output - High drive Status
//   P1.5 - PCTRL Digital Input with internal pull-up - Power Control configuration
//          1: Power switch on whenever battery voltage good
//          0: Power switched on only at night when battery voltage good
//   P1.6 - Reserved for 32kHz crystal (used for diagnostic output)
//   P1.7 - Reserved for 32kHz crystal (used for diagnostic output)
//
// Peripheral Utilization:
//   ADC0 - Voltage, Current, Temperature measurements
//   TIMER0 - ADC sampling trigger (~250 uSec)
//   TIMER2 - Main loop evaluation control
//   SMB0 - I2C interface
//   PCA0 - MPPT Buck PWM output, Indicator LED PWM output,
//          internal watchdog
//
//
// SolarMpptCharger is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SolarMpptCharger is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// See <http://www.gnu.org/licenses/>.
//
//=========================================================

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SI_EFM8SB1_Register_Enums.h>                  // SFR declarations
#include "InitDevice.h"
#include "adc.h"
#include "buck.h"
#include "charge.h"
#include "config.h"
#include "led.h"
#include "param.h"
#include "power.h"
#include "smbus.h"
#include "temp.h"
#include "timer.h"
#include "watchdog.h"

// $[Generated Includes]
// [Generated Includes]$

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------
// Main evaluation control variables
uint8_t mainEvalPhase = 0;

//-----------------------------------------------------------------------------
// SiLabs_Startup() Routine
// ----------------------------------------------------------------------------
// This function is called immediately after reset, before the initialization
// code is run in SILABS_STARTUP.A51 (which runs before main() ). This is a
// useful place to disable the watchdog timer, which is enable by default
// and may trigger before main() in some instances.
//-----------------------------------------------------------------------------
void SiLabs_Startup(void) {
	WD_Disable();
}

//-----------------------------------------------------------------------------
// main() Routine
// ----------------------------------------------------------------------------
int main(void) {
	// Call hardware initialization routine
	enter_DefaultMode_from_RESET();

	// System initialization
	PARAM_Init();  // Initialize first to load operating params
	ADC_Init();    // Initialize second to get initial external values
	BUCK_Init();
	CHARGE_Init();
	POWER_Init();
	TEMP_Init();
	WD_Init();
	LED_Init();    // Initialize after other state generating modules
	SMB_Init();    // Initialize after other state generating modules
	TIMER_Init();  // Initialize last just prior to starting main loop

	while (1) {
		// Clear watchdog
		WD_Reset();

		// Update scheduling timer
		TIMER_Update();

		// 10 mSec activities
		if (TIMER_FastTick()) {
			LED_Update();
		}

		// 250 mSec or slower activities
		//  Note: code execution through this path measured at 354 - 926 uSec
		if (TIMER_SlowTick()) {
			CHARGE_MpptUpdate();

			// Once per second activities
			switch (mainEvalPhase) {
			case 0:
				TEMP_Update();
				break;
			case 1:
				CHARGE_StateUpdate();
				break;
			case 2:
				POWER_Update();
				break;
			}
			if (++mainEvalPhase == 4)
				mainEvalPhase = 0;
		}
	}
}

// $[Generated Run-time code]
// [Generated Run-time code]$

// $[SiLabs Startup]
// [SiLabs Startup]$

