/*****************************************************************************
* File Name: CY8CMBR3xxx_HostFunctions.c
*
* Version 1.00
*
* Description:
*   This file contains the definitions of the low-level APIs. You may need to 
*   modify the content of these APIs to suit your host processor’s I2C 
*   implementation.
*
* Note:
*   These host-dependent Low Level APIs are provided as an example of
*   low level I2C read and write functions. This set of low level APIs are 
*   written for PSoC 4200/4100 devices and hence should be re-written
*   with equivalent host-dependent APIs from the respective IDEs, if the 
*   host design does not include PSoC 4200/4100 device.
* 
*   To use these APIs, the host should implement a working I2C communication
*   interface. This interface will be used by these APIs to communicate to the
*   CY8CMBR3xxx device.
*
*   For PSoC 4200/4100 devices, please ensure that you have created an instance 
*   of SCB component with I2C Master configuration. The component should be
*   named "SCB".
*
* Owner:
*   SRVS
*
* Related Document:
*   MBR3 Design Guide
*   MBR3 Device Datasheet
*
* Hardware Dependency:
*   PSoC 4200 (Update this as per the host used)
*
* Code Tested With:
*   PSoC Creator 3.0 CP7
*   CY3280-MBR3 Evaluation Kit
*   CY8CKIT-042 Pioneer Kit
*
******************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/

/*******************************************************************************
* Included headers
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "driver/adc.h"

#include "event-control-platform.h"
#include "event-control.h"

#include "button_adc_platform.h"
#include "button_adc.h"

/*******************************************************************************
* API Constants
*******************************************************************************/
//#define HAL_BUTTON_ADC_PLATFORM_READ_USE_TIMER

/*******************************************************************************
*   Function Code
*******************************************************************************/

static uint16_t buttonAdcbuttonChangedEventNumber;

#ifdef HAL_BUTTON_ADC_PLATFORM_READ_USE_TIMER
static void readAdcEventHandler(xTimerHandle xTimer)
#else
static uint16_t buttonAdcreadAdcEventNumber;
static void readAdcEventHandler(void)
#endif
{
	uint32_t buttonAdcValue;
	uint16_t adc_data[4];
	uint8_t i;

#ifndef HAL_BUTTON_ADC_PLATFORM_READ_USE_TIMER
	eventControlSetDelayMS(buttonAdcreadAdcEventNumber, HAL_BUTTON_ADC_PERIOD);
#endif

	if (ESP_OK != adc_read_fast(adc_data, sizeof(adc_data)/sizeof(uint16_t)))
	{
		return;
	}

	for(buttonAdcValue=0,i=0;i<(sizeof(adc_data)/sizeof(uint16_t));i++)
	{
		buttonAdcValue += adc_data[i];
	}
	halButtonAdcReadEventHandler(0, buttonAdcValue/(sizeof(adc_data)/sizeof(uint16_t)));
}

#ifndef HAL_BUTTON_ADC_PLATFORM_READ_USE_TIMER
static const EventControl_t readAdcEventControl =
{
	.name = "button-adc-read",
	.callback = (EventControlCallback_t)readAdcEventHandler,
	.args = NULL
};
#endif

static const EventControl_t buttonChangedEventControl =
{
	.name = "button-changed",
	.callback = (EventControlCallback_t)halButtonAdcButtonChangeEventHandler,
	.args = NULL
};


ButtonAdcPlatformTick_t halButtonAdcPlatformGetTick(void)
{
	return xTaskGetTickCount();
//	return clock();
}

uint32_t halButtonAdcPlatformTick2MS(ButtonAdcPlatformTick_t tick)
{
	// portTICK_RATE_MS: milliseconds per tick
	return ((uint32_t)tick * portTICK_RATE_MS);
//	return (tick * (1000 / CLOCKS_PER_SEC));
}

void buttonAdcPlatformEventControlSetActive(ButtonAdcEvent_t evt)
{
	switch(evt)
	{
	case ButtonAdcEvent_READ_ADC:
#ifndef HAL_BUTTON_ADC_PLATFORM_READ_USE_TIMER
		eventControlSetActive(buttonAdcreadAdcEventNumber);
#endif
		break;
	case ButtonAdcEvent_BUTTON_CHANGED:
		eventControlSetActive(buttonAdcbuttonChangedEventNumber);
		break;
	}
}

void buttonAdcPlatformEventControlSetDelayMS(ButtonAdcEvent_t evt, uint16_t timerPeriodMs)
{
	switch(evt)
	{
	case ButtonAdcEvent_READ_ADC:
#ifndef HAL_BUTTON_ADC_PLATFORM_READ_USE_TIMER
		eventControlSetDelayMS(buttonAdcreadAdcEventNumber, timerPeriodMs);
#endif
		break;
	case ButtonAdcEvent_BUTTON_CHANGED:
		eventControlSetDelayMS(buttonAdcbuttonChangedEventNumber, timerPeriodMs);
		break;
	}
}

void buttonAdcPlatformEventControlSetInactive(ButtonAdcEvent_t evt)
{
	switch(evt)
	{
	case ButtonAdcEvent_READ_ADC:
#ifndef HAL_BUTTON_ADC_PLATFORM_READ_USE_TIMER
		eventControlSetInactive(buttonAdcreadAdcEventNumber);
#endif
		break;
	case ButtonAdcEvent_BUTTON_CHANGED:
		eventControlSetInactive(buttonAdcbuttonChangedEventNumber);
		break;
	}
}

void halButtonAdcPlatformInit(void)
{
	// read ADC
#ifdef HAL_BUTTON_ADC_PLATFORM_READ_USE_TIMER
	portBASE_TYPE xReturn;

	xTimerHandle readAdcTimer = xTimerCreate(	"button-adc-read",
												HAL_BUTTON_ADC_PERIOD/portTICK_RATE_MS,
												pdTRUE,
												NULL,
												readAdcEventHandler );
	configASSERT(readAdcTimer);
	xReturn = xTimerStart(readAdcTimer, portMAX_DELAY);
	configASSERT(xReturn);
#else
	buttonAdcreadAdcEventNumber = eventControlRegister(&readAdcEventControl);
	eventControlSetDelayMS(buttonAdcreadAdcEventNumber, HAL_BUTTON_ADC_PERIOD);
#if defined(HAL_BUTTON_ADC_DEBUG_ENABLED) && defined(EVENT_CONTROL_DEBUG_ENABLED)
//	eventControlDebugEnable(buttonAdcreadAdcEventNumber);
#endif
#endif
	buttonAdcbuttonChangedEventNumber = eventControlRegister(&buttonChangedEventControl);
#if defined(HAL_BUTTON_ADC_DEBUG_ENABLED) && defined(EVENT_CONTROL_DEBUG_ENABLED)
	eventControlDebugEnable(buttonAdcbuttonChangedEventNumber);
#endif
}

/****************************End of File***************************************/
