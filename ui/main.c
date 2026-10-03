/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "app/auscb.h"
#include <string.h>
#include <stdlib.h>  // abs()

#include "app/chFrScanner.h"
#include "app/dtmf.h"
#ifdef ENABLE_AM_FIX
	#include "am_fix.h"
#endif
#include "bitmaps.h"
#include "board.h"
#include "driver/bk4819.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/main.h"
#include "ui/ui.h"

center_line_t center_line = CENTER_LINE_NONE;

const int8_t dBmCorrTable[7] = {
			-15, // band 1
			-25, // band 2
			-20, // band 3
			-4, // band 4
			-7, // band 5
			-6, // band 6
			 -1  // band 7
		};

const char *VfoStateStr[] = {
       [VFO_STATE_NORMAL]="",
       [VFO_STATE_BUSY]="BUSY",
       [VFO_STATE_BAT_LOW]="BAT LOW",
       [VFO_STATE_TX_DISABLE]="TX DISABLE",
       [VFO_STATE_TIMEOUT]="TIMEOUT",
       [VFO_STATE_ALARM]="ALARM",
       [VFO_STATE_VOLTAGE_HIGH]="VOLT HIGH"
};

// ***************************************************************************

static void DrawSmallAntennaAndBars(uint8_t *p, unsigned int level)
{
	if(level>6)
		level = 6;

	memcpy(p, BITMAP_Antenna, ARRAY_SIZE(BITMAP_Antenna));

	for(uint8_t i = 1; i <= level; i++) {
		char bar = (0xff << (6-i)) & 0x7F;
		memset(p + 2 + i*3, bar, 2);
	}
}
#if defined ENABLE_AUDIO_BAR || defined ENABLE_RSSI_BAR

static void DrawLevelBar(uint8_t xpos, uint8_t line, uint8_t level)
{
	const char hollowBar[] = {
		0b01111111,
		0b01000001,
		0b01000001,
		0b01111111
	};

	uint8_t *p_line = gFrameBuffer[line];
	level = MIN(level, 13);

	for(uint8_t i = 0; i < level; i++) {
		if(i < 9) {
			for(uint8_t j = 0; j < 4; j++)
				p_line[xpos + i * 5 + j] = (~(0x7F >> (i+1))) & 0x7F;
		}
		else {
			memcpy(p_line + (xpos + i * 5), &hollowBar, ARRAY_SIZE(hollowBar));
		}
	}
}
#endif

#ifdef ENABLE_AUDIO_BAR

unsigned int sqrt16(unsigned int value)
{	// return square root of 'value'
	unsigned int shift = 16;         // number of bits supplied in 'value' .. 2 ~ 32
	unsigned int bit   = 1u << --shift;
	unsigned int sqrti = 0;
	while (bit)
	{
		const unsigned int temp = ((sqrti << 1) | bit) << shift--;
		if (value >= temp) {
			value -= temp;
			sqrti |= bit;
		}
		bit >>= 1;
	}
	return sqrti;
}

void UI_DisplayAudioBar(void)
{
	if (gSetting_mic_bar)
	{
		if(gLowBattery && !gLowBatteryConfirmed)
			return;

		const unsigned int line      = 3;

		if (gCurrentFunction != FUNCTION_TRANSMIT ||
			gScreenToDisplay != DISPLAY_MAIN
#ifdef ENABLE_DTMF_CALLING
			|| gDTMF_CallState != DTMF_CALL_STATE_NONE
#endif
			)
		{
			return;  // screen is in use
		}

#if defined(ENABLE_ALARM) || defined(ENABLE_TX1750)
		if (gAlarmState != ALARM_STATE_OFF)
			return;
#endif
		const unsigned int voice_amp  = BK4819_GetVoiceAmplitudeOut();  // 15:0

		// make non-linear to make more sensitive at low values
		const unsigned int level      = MIN(voice_amp * 8, 65535u);
		const unsigned int sqrt_level = MIN(sqrt16(level), 124u);
		uint8_t bars = 13 * sqrt_level / 124;

		uint8_t *p_line = gFrameBuffer[line];
		memset(p_line, 0, LCD_WIDTH);

		DrawLevelBar(62, line, bars);

		if (gCurrentFunction == FUNCTION_TRANSMIT)
			ST7565_BlitFullScreen();
	}
}
#endif


void DisplayRSSIBar(const bool now)
{
#if defined(ENABLE_RSSI_BAR)

	const unsigned int txt_width    = 7 * 8;                 // 8 text chars
	const unsigned int bar_x        = 2 + txt_width + 4;     // X coord of bar graph

	const unsigned int line         = 3;
	uint8_t           *p_line        = gFrameBuffer[line];
	char               str[16];

	const char plus[] = {
		0b00011000,
		0b00011000,
		0b01111110,
		0b01111110,
		0b01111110,
		0b00011000,
		0b00011000,
	};

	if ((gEeprom.KEY_LOCK && gKeypadLocked > 0) || center_line != CENTER_LINE_RSSI)
		return;     // display is in use

	if (gCurrentFunction == FUNCTION_TRANSMIT ||
		gScreenToDisplay != DISPLAY_MAIN
#ifdef ENABLE_DTMF_CALLING
		|| gDTMF_CallState != DTMF_CALL_STATE_NONE
#endif
		)
		return;     // display is in use

	if (now)
		memset(p_line, 0, LCD_WIDTH);


	const int16_t s0_dBm   = -gEeprom.S0_LEVEL;                  // S0 .. base level
	const int16_t rssi_dBm =
		BK4819_GetRSSI_dBm()
#ifdef ENABLE_AM_FIX
		+ ((gSetting_AM_fix && gRxVfo->Modulation == MODULATION_AM) ? AM_fix_get_gain_diff() : 0)
#endif
		+ dBmCorrTable[gRxVfo->Band];

	int s0_9 = gEeprom.S0_LEVEL - gEeprom.S9_LEVEL;
	const uint8_t s_level = MIN(MAX((int32_t)(rssi_dBm - s0_dBm)*100 / (s0_9*100/9), 0), 9); // S0 - S9
	uint8_t overS9dBm = MIN(MAX(rssi_dBm + gEeprom.S9_LEVEL, 0), 99);
	uint8_t overS9Bars = MIN(overS9dBm/10, 4);

	if(overS9Bars == 0) {
		sprintf(str, "% 4d S%d", rssi_dBm, s_level);
	}
	else {
		sprintf(str, "% 4d  %2d", rssi_dBm, overS9dBm);
		memcpy(p_line + 2 + 7*5, &plus, ARRAY_SIZE(plus));
	}

	UI_PrintStringSmallNormal(str, 2, 0, line);
	DrawLevelBar(bar_x, line, s_level + overS9Bars);
	if (now)
		ST7565_BlitLine(line);
#else
	int16_t rssi = BK4819_GetRSSI();
	uint8_t Level;

	if (rssi >= gEEPROM_RSSI_CALIB[gRxVfo->Band][3]) {
		Level = 6;
	} else if (rssi >= gEEPROM_RSSI_CALIB[gRxVfo->Band][2]) {
		Level = 4;
	} else if (rssi >= gEEPROM_RSSI_CALIB[gRxVfo->Band][1]) {
		Level = 2;
	} else if (rssi >= gEEPROM_RSSI_CALIB[gRxVfo->Band][0]) {
		Level = 1;
	} else {
		Level = 0;
	}

	uint8_t *pLine = (gEeprom.RX_VFO == 0)? gFrameBuffer[2] : gFrameBuffer[6];
	if (now)
		memset(pLine, 0, 23);
	DrawSmallAntennaAndBars(pLine, Level);
	if (now)
		ST7565_BlitFullScreen();
#endif

}

#ifdef ENABLE_AGC_SHOW_DATA
void UI_MAIN_PrintAGC(bool now)
{
	char buf[20];
	memset(gFrameBuffer[3], 0, 128);
	union {
		struct {
			uint16_t _ : 5;
			uint16_t agcSigStrength : 7;
			int16_t gainIdx : 3;
			uint16_t agcEnab : 1;
		};
    	uint16_t __raw;
	} reg7e;
	reg7e.__raw = BK4819_ReadRegister(0x7E);
	uint8_t gainAddr = reg7e.gainIdx < 0 ? 0x14 : 0x10 + reg7e.gainIdx;
	union {
		struct {
			uint16_t pga:3;
			uint16_t mixer:2;
			uint16_t lna:3;
			uint16_t lnaS:2;
		};
		uint16_t __raw;
	} agcGainReg;
	agcGainReg.__raw = BK4819_ReadRegister(gainAddr);
	int8_t lnaShortTab[] = {-28, -24, -19, 0};
	int8_t lnaTab[] = {-24, -19, -14, -9, -6, -4, -2, 0};
	int8_t mixerTab[] = {-8, -6, -3, 0};
	int8_t pgaTab[] = {-33, -27, -21, -15, -9, -6, -3, 0};
	int16_t agcGain = lnaShortTab[agcGainReg.lnaS] + lnaTab[agcGainReg.lna] + mixerTab[agcGainReg.mixer] + pgaTab[agcGainReg.pga];

	sprintf(buf, "%d%2d %2d %2d %3d", reg7e.agcEnab, reg7e.gainIdx, -agcGain, reg7e.agcSigStrength, BK4819_GetRSSI());
	UI_PrintStringSmallNormal(buf, 2, 0, 3);
	if(now)
		ST7565_BlitLine(3);
}
#endif

void UI_MAIN_TimeSlice500ms(void)
{
	if(gScreenToDisplay==DISPLAY_MAIN) {
#ifdef ENABLE_AGC_SHOW_DATA
		UI_MAIN_PrintAGC(true);
		return;
#endif

		if(FUNCTION_IsRx()) {
			DisplayRSSIBar(true);
		}
	}
}

// ***************************************************************************

void UI_DisplayMain(void)
{
	char               String[22];

	center_line = CENTER_LINE_NONE;

	// clear the screen
	UI_DisplayClear();

	if(gLowBattery && !gLowBatteryConfirmed) {
		UI_DisplayPopup("LOW BATTERY");
		ST7565_BlitFullScreen();
		return;
	}

	if (gEeprom.KEY_LOCK && gKeypadLocked > 0)
	{	// tell user how to unlock the keyboard
		UI_PrintString("Long press #", 0, LCD_WIDTH, 1, 8);
		UI_PrintString("to unlock",    0, LCD_WIDTH, 3, 8);
		ST7565_BlitFullScreen();
		return;
	}

	unsigned int activeTxVFO = gRxVfoIsActive ? gEeprom.RX_VFO : gEeprom.TX_VFO;

    if (gAusCbMode && gAircraftMode) {
        const AirAirport *airport = AIR_GetAirport(gAirAirport);
        const AirChannel *channel = AIR_GetChannel();
        sprintf(String, "%s %s", gAirRegions[gAirRegion], airport->ident);
        UI_PrintStringSmallBold(String, 2, 0, 0);
        UI_PrintStringSmallBold(FUNCTION_IsRx() ? "RX" : "", 108, 0, 0);
        UI_PrintStringSmallBold(airport->name, 0, LCD_WIDTH, 1);
        UI_PrintStringSmallNormal(gAirServices[channel->service], 0, LCD_WIDTH, 2);
        sprintf(String, "%lu.%05lu", (unsigned long)(channel->frequency/100000), (unsigned long)(channel->frequency%100000));
        UI_PrintString(String, 0, LCD_WIDTH, 4, 8);
        UI_PrintStringSmallNormal(gScanStateDir != SCAN_OFF ? "SCANNING / RX ONLY" : "AIRCRAFT / RX ONLY", 0, LCD_WIDTH, 6);
#ifdef ENABLE_RSSI_BAR
        if (FUNCTION_IsRx()) { center_line=CENTER_LINE_RSSI; DisplayRSSIBar(false); }
#endif
        ST7565_BlitFullScreen();
        return;
    }
	if (gAusCbMode) {
		// A single CB view: channel on pages 1-2, live meter on page 3.
		const bool transmitting = gCurrentFunction == FUNCTION_TRANSMIT;
		UI_PrintStringSmallBold(transmitting ? "TX" : FUNCTION_IsRx() ? "RX" : "", 108, 0, 0);
		sprintf(String, "CB %02u", gAusCbChannel);
		UI_PrintString(String, 0, LCD_WIDTH, 1, 8);
		const char *power[] = {"LOW", "MID", "HIGH"};
		sprintf(String, "%s  NFM%s", power[gCurrentVfo->OUTPUT_POWER % 3],
		        gAusCbDuplex && AUSCB_IsRepeater(gAusCbChannel) ? "  DUP" : "");
		UI_PrintStringSmallNormal(String, 2, 0, 0);
		UI_PrintStringSmallBold(AUSCB_ChannelUse(gAusCbChannel), 0, LCD_WIDTH, 4);
		const enum VfoState_t state = VfoState[activeTxVFO];
		if (state != VFO_STATE_NORMAL && state < ARRAY_SIZE(VfoStateStr))
			UI_PrintStringSmallBold(VfoStateStr[state], 0, LCD_WIDTH, 5);
		else if (!AUSCB_VoiceChannel(gAusCbChannel))
			UI_PrintStringSmallBold("RX ONLY", 0, LCD_WIDTH, 5);
		if (gScanStateDir != SCAN_OFF)
			UI_PrintStringSmallNormal("SCANNING", 0, LCD_WIDTH, 6);
#ifdef ENABLE_AUDIO_BAR
		if (transmitting && gSetting_mic_bar) {
			center_line = CENTER_LINE_AUDIO_BAR;
			UI_DisplayAudioBar();
		}
#endif
#ifdef ENABLE_RSSI_BAR
		if (FUNCTION_IsRx()) {
			center_line = CENTER_LINE_RSSI;
			DisplayRSSIBar(false);
		}
#endif
		ST7565_BlitFullScreen();
		return;
	}

	ST7565_BlitFullScreen();
}
