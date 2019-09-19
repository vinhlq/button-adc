/*****************************************************************************
* File Name: button_adc.c
*
* Version 1.00
*
* Description:
*   This file contains the declarations of all the high-level APIs.
*
* Note:
*   N/A
*
* Owner:
*   vinhlq
*
* Related Document:
*
* Hardware Dependency:
*   N/A
*
* Code Tested With:
*
******************************************************************************
* Copyright (2019), vinhlq.
******************************************************************************
* This software is owned by vinhlq and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* (vinhlq) hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* (vinhlq) Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a (vinhlq) integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of (vinhlq).
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* (vinhlq) reserves the right to make changes without further notice to the
* materials described herein. (vinhlq) does not assume any liability arising out
* of the application or use of any product or circuit described herein. (vinhlq)
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of (vinhlq)' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies (vinhlq) against all charges. Use may be
* limited by and subject to the applicable (vinhlq) software license agreement.
*****************************************************************************/

#include <stdint.h>
#include <string.h>

#include "button_adc_platform.h"
#include "button_adc.h"

#ifdef HAL_BUTTON_ADC_DEBUG_ENABLED
#define debugPrintln(fmt,args...)	printf(fmt "%s", ## args, "\r\n")
#else
#define debugPrintln(...)
#endif

#define HAL_BUTTON_ADC_ENABLED					(1<<0)
#define HAL_BUTTON_ADC_PRESSING					(1<<2)
#define HAL_BUTTON_ADC_EVENT_PRESSING			(1<<3)
#define HAL_BUTTON_ADC_EVENT_PRESS0				(1<<4)
#define HAL_BUTTON_ADC_EVENT_PRESS1				(1<<5)
#define HAL_BUTTON_ADC_EVENT_PRESS				(	HAL_BUTTON_ADC_EVENT_PRESSING |	\
													HAL_BUTTON_ADC_EVENT_PRESS0 |	\
													HAL_BUTTON_ADC_EVENT_PRESS1)
#define HAL_BUTTON_ADC_EVENT_PRESSING_TIMEOUT	HAL_BUTTON_ADC_EVENT_PRESS0
#define HAL_BUTTON_ADC_EVENT_PRESS_SHORT		HAL_BUTTON_ADC_EVENT_PRESS1
#define HAL_BUTTON_ADC_EVENT_PRESS_LONG			(HAL_BUTTON_ADC_EVENT_PRESS0 | HAL_BUTTON_ADC_EVENT_PRESS1)
#define HAL_BUTTON_ADC_EVENT_ALL			(	HAL_BUTTON_ADC_PRESSING |			\
											HAL_BUTTON_ADC_EVENT_PRESS0	|			\
											HAL_BUTTON_ADC_EVENT_PRESS1)

#define allBitsIsSet(status, bits)	((status & (bits)) == (bits))
#define setBitsWithMask(reg, bits, mask)	(reg = ((reg & ~(mask)) | (bits)))

static ButtonAdcPlatformTick_t buttonAdcPressingTicks[HAL_BUTTON_ADC_COUNT];
static uint16_t buttonAdcTimePressedMs[HAL_BUTTON_ADC_COUNT];
static uint8_t buttonAdcSatus[HAL_BUTTON_ADC_COUNT];

static ButtonAdcADCValue_t buttonAdcValues[HAL_BUTTON_ADC_COUNT];
static ButtonAdcADCValue_t buttonAdcThreshold[HAL_BUTTON_ADC_COUNT];
static uint8_t buttonAdcThresholdError[HAL_BUTTON_ADC_COUNT];

static uint8_t halButtonAdcIsPressed(uint8_t index, ButtonAdcADCValue_t buttonAdcValue)
{
	ButtonAdcADCValue_t thresholdTemp;

	if(buttonAdcThreshold[index] + buttonAdcThresholdError[index] < HAL_BUTTON_ADC_VALUE_MAX)
	{
		thresholdTemp = buttonAdcThreshold[index] + buttonAdcThresholdError[index];
	}
	else
	{
		thresholdTemp = HAL_BUTTON_ADC_VALUE_MAX;
	}
	// check upper
	if(buttonAdcValue > thresholdTemp)
	{
		return 0;
	}
	if(buttonAdcThreshold[index] > (HAL_BUTTON_ADC_VALUE_MIN + buttonAdcThresholdError[index]))
	{
		thresholdTemp = buttonAdcThreshold[index] - buttonAdcThresholdError[index];
	}
	else
	{
		thresholdTemp = HAL_BUTTON_ADC_VALUE_MIN;
	}
	// check lower
	if(buttonAdcValue < thresholdTemp)
	{
		return 0;
	}
	return 1;
}

#if HAL_BUTTON_ADC_COUNT == 1
#define getNextButtonIndex(index) (0)
#define getPrevButtonIndex(index) (0)
#else
#define getNextButtonIndex(index)	(index < (HAL_BUTTON_ADC_COUNT-1) ? (index + 1):0)
#define getPrevButtonIndex(index)	(index > 0 ? (index-1):(HAL_BUTTON_ADC_COUNT-1))
#endif
void halButtonAdcButtonChangeEventHandler(void)
{
	static uint8_t b_index=0;
	uint8_t b_index_last;

	buttonAdcPlatformEventControlSetInactive(ButtonAdcEvent_BUTTON_CHANGED);

	b_index_last=getPrevButtonIndex(b_index);
	do{
		uint8_t status = buttonAdcSatus[b_index];
		if(status & HAL_BUTTON_ADC_ENABLED)
		{
			if(allBitsIsSet(status, HAL_BUTTON_ADC_PRESSING | HAL_BUTTON_ADC_EVENT_PRESSING))
			{
				buttonAdcSatus[b_index] &= ~HAL_BUTTON_ADC_EVENT_PRESSING;
				if(allBitsIsSet(status, HAL_BUTTON_ADC_EVENT_PRESSING_TIMEOUT))
				{
#ifdef HAL_BUTTON_ADC_PRESSING_TIMEOUT_CALLBACK
					halButtonAdcPressingTimeoutCallback(b_index, buttonAdcValues[b_index], buttonAdcTimePressedMs[b_index]);
#endif
				}
				else
				{
#ifdef HAL_BUTTON_ADC_PRESSING_CALLBACK
					halButtonAdcPressingCallback(b_index, buttonAdcValues[b_index]);
#endif
				}
				// release CPU for other event handler
				b_index = getNextButtonIndex(b_index);
				break;
			}
			else
			{
#ifdef HAL_BUTTON_ADC_RELEASE_CALLBACK
				halButtonAdcReleaseCallback(b_index, buttonAdcValues[b_index], buttonAdcTimePressedMs[b_index]);
#endif

				if(allBitsIsSet(status, HAL_BUTTON_ADC_EVENT_PRESS_LONG))
				{
					buttonAdcSatus[b_index] &= ~HAL_BUTTON_ADC_EVENT_PRESS_LONG;
#ifdef HAL_BUTTON_ADC_PRESS_LONG_CALLBACK
					halButtonAdcPressLongCallback(b_index, buttonAdcTimePressedMs[b_index]);
#endif
					// release CPU for other event handler
					b_index = getNextButtonIndex(b_index);
					break;
				}
				else if(status & HAL_BUTTON_ADC_EVENT_PRESS_SHORT)
				{
					buttonAdcSatus[b_index] &= ~HAL_BUTTON_ADC_EVENT_PRESS_SHORT;
#ifdef HAL_BUTTON_ADC_PRESS_SHORT_CALLBACK
					halButtonAdcPressShortCallback(b_index, buttonAdcTimePressedMs[b_index]);
#endif
					// release CPU for other event handler
					b_index = getNextButtonIndex(b_index);
					break;
				}
			}
		}


		// next button
		b_index = getNextButtonIndex(b_index);
	} while(b_index != b_index_last);


	if(b_index != b_index_last)
	{
		buttonAdcPlatformEventControlSetActive(ButtonAdcEvent_BUTTON_CHANGED);
	}
}

void halButtonAdcReadEventHandler(uint8_t index, ButtonAdcADCValue_t buttonAdcValue)
{
	ButtonAdcPlatformTick_t currentTick;
	uint16_t timePressedMs;
	uint8_t status;

	if(index >= HAL_BUTTON_ADC_COUNT)
	{
		return;
	}

	if(!(buttonAdcSatus[index] & HAL_BUTTON_ADC_ENABLED))
	{
		return;
	}



	currentTick = halButtonAdcPlatformGetTick();
	status = buttonAdcSatus[index];
	if(halButtonAdcIsPressed(index, buttonAdcValue))
	{
		if(!(status & HAL_BUTTON_ADC_PRESSING))
		{
			buttonAdcValues[index] = buttonAdcValue;
			buttonAdcPressingTicks[index] = currentTick;
			buttonAdcSatus[index] |= (	HAL_BUTTON_ADC_PRESSING |
										HAL_BUTTON_ADC_EVENT_PRESSING);
			buttonAdcPlatformEventControlSetActive(ButtonAdcEvent_BUTTON_CHANGED);
		}
		else if(!(status & HAL_BUTTON_ADC_EVENT_PRESSING_TIMEOUT))
		{
			timePressedMs = halButtonAdcPlatformTick2MS(currentTick - buttonAdcPressingTicks[index]);
			if(timePressedMs > HAL_BUTTON_ADC_PRESSING_TIMEOUT_PERIOD)
			{
				setBitsWithMask
					(
						buttonAdcSatus[index],
						HAL_BUTTON_ADC_EVENT_PRESSING | HAL_BUTTON_ADC_EVENT_PRESSING_TIMEOUT,
						HAL_BUTTON_ADC_EVENT_PRESS
					);
				buttonAdcPlatformEventControlSetActive(ButtonAdcEvent_BUTTON_CHANGED);
			}
		}
	}
	else
	{
		buttonAdcSatus[index] &= ~(HAL_BUTTON_ADC_PRESSING | HAL_BUTTON_ADC_EVENT_PRESSING | HAL_BUTTON_ADC_EVENT_PRESSING_TIMEOUT);
		if(status & HAL_BUTTON_ADC_PRESSING)
		{
			buttonAdcValues[index] = buttonAdcValue;

			timePressedMs = halButtonAdcPlatformTick2MS(currentTick - buttonAdcPressingTicks[index]);
			buttonAdcTimePressedMs[index] = timePressedMs;
			if(timePressedMs > HAL_BUTTON_ADC_PRESS_LONG_PERIOD)
			{
				setBitsWithMask
					(
						buttonAdcSatus[index],
						HAL_BUTTON_ADC_EVENT_PRESS_LONG,
						HAL_BUTTON_ADC_EVENT_PRESS
					);
			}
			else if(timePressedMs > HAL_BUTTON_ADC_PRESS_SHORT_PERIOD)
			{
				setBitsWithMask
					(
						buttonAdcSatus[index],
						HAL_BUTTON_ADC_EVENT_PRESS_SHORT,
						HAL_BUTTON_ADC_EVENT_PRESS
					);
			}
			buttonAdcPlatformEventControlSetActive(ButtonAdcEvent_BUTTON_CHANGED);
		}
	}
}

void halButtonAdcInit(void)
{
	uint8_t i;

	for(i = 0; i < HAL_BUTTON_ADC_COUNT; i++)
	{
		buttonAdcPressingTicks[i] = portMAX_DELAY;
		buttonAdcSatus[i] = 0;
	}
}

void halButtonAdcEnable(uint8_t index, ButtonAdcADCValue_t threshold, uint8_t error)
{
	if(index >= HAL_BUTTON_ADC_COUNT)
	{
		return;
	}
	buttonAdcSatus[index] |= HAL_BUTTON_ADC_ENABLED;
	buttonAdcThreshold[index] = threshold;
	buttonAdcThresholdError[index] = error;
}


/*
 * EOF
 */

