/*****************************************************************************
* File Name: button_adc.h
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

/*******************************************************************************
* Included headers
*******************************************************************************/

/*******************************************************************************
* User defined Macros
*******************************************************************************/

#ifndef BUTTON_ADC_H
#define BUTTON_ADC_H

#ifndef HAL_BUTTON_ADC_COUNT
#define HAL_BUTTON_ADC_COUNT (1)
#endif

#ifndef HAL_BUTTON_ADC_PERIOD
#define HAL_BUTTON_ADC_PERIOD				(50)
#endif

#ifndef HAL_BUTTON_ADC_VALUE_MAX
#define HAL_BUTTON_ADC_VALUE_MAX	(0x3ff)
#endif

#ifndef HAL_BUTTON_ADC_VALUE_MIN
#define HAL_BUTTON_ADC_VALUE_MIN	(0)
#endif

#ifndef HAL_BUTTON_ADC_PRESSING_TIMEOUT_PERIOD
#define HAL_BUTTON_ADC_PRESSING_TIMEOUT_PERIOD	(3000)
#endif

#ifndef HAL_BUTTON_ADC_PRESS_SHORT_PERIOD
#define HAL_BUTTON_ADC_PRESS_SHORT_PERIOD	(150)
#endif

#ifndef HAL_BUTTON_ADC_PRESS_LONG_PERIOD
#define HAL_BUTTON_ADC_PRESS_LONG_PERIOD	(2000)
#endif

/*******************************************************************************
* Structure Definitions
*******************************************************************************/

typedef enum
{
	ButtonAdcEvent_READ_ADC,
	ButtonAdcEvent_BUTTON_CHANGED,
}ButtonAdcEvent_t;

/*******************************************************************************
* Function callbacks
*******************************************************************************/

/** @brief halButtonAdcPressShortCallback
 *
 *
 * @param index  			Ver.: always
 * @param timePressedMs  	Ver.: always
 */
void halButtonAdcPressShortCallback(uint8_t index, uint16_t timePressedMs);

/** @brief halButtonAdcPressLongCallback
 *
 *
 * @param index  			Ver.: always
 * @param timePressedMs  	Ver.: always
 */
void halButtonAdcPressLongCallback(uint8_t index, uint16_t timePressedMs);

/** @brief halButtonAdcPressingCallback
 *
 *
 * @param index  Ver.: always
 */
void halButtonAdcPressingCallback(uint8_t index, ButtonAdcADCValue_t value);

/** @brief halButtonAdcPressingTimeout
 *
 *
 * @param index  Ver.: always
 */
void halButtonAdcPressingTimeoutCallback(uint8_t index, ButtonAdcADCValue_t value, uint16_t timePressedMs);

/** @brief halButtonAdcReleaseCallback
 *
 *
 * @param index  Ver.: always
 */
void halButtonAdcReleaseCallback(uint8_t index, ButtonAdcADCValue_t value, uint16_t timePressedMs);

/*******************************************************************************
* Function Event handlers
*******************************************************************************/

/** @brief halButtonAdcReadEventHandler
 *
 *
 * @param index  Ver.: always
 */
void halButtonAdcReadEventHandler(uint8_t index, ButtonAdcADCValue_t buttonAdcValue);

/** @brief halButtonAdcButtonChangeEventHandler
 *
 *
 * @param index  Ver.: always
 */
void halButtonAdcButtonChangeEventHandler(void);

/*******************************************************************************
* Function Platform Implements
*******************************************************************************/

ButtonAdcPlatformTick_t halButtonAdcPlatformGetTick(void);

uint32_t halButtonAdcPlatformTick2MS(ButtonAdcPlatformTick_t tick);

void buttonAdcPlatformEventControlSetActive(ButtonAdcEvent_t evt);
void buttonAdcPlatformEventControlSetDelayMS(ButtonAdcEvent_t evt, uint16_t timerPeriodMs);
void buttonAdcPlatformEventControlSetInactive(ButtonAdcEvent_t evt);

/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** @brief halButtonAdcEnable
 *
 *
 * @param index  Ver.: always
 */
void halButtonAdcEnable(uint8_t index, ButtonAdcADCValue_t threshold, uint8_t error);

/** @brief halButtonAdcInit
 *
 *
 * @param index  Ver.: always
 */
void halButtonAdcInit(void);

#endif
