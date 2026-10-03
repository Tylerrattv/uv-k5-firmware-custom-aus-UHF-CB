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

#include "app/action.h"
#include "app/app.h"
#include "app/chFrScanner.h"
#include "app/common.h"
#ifdef ENABLE_FMRADIO
	#include "app/fm.h"
#endif
#include "app/generic.h"
#include "app/main.h"
#include "app/scanner.h"

#ifdef ENABLE_SPECTRUM
#include "app/spectrum.h"
#endif

#include "audio.h"
#include "board.h"
#include "driver/bk4819.h"
#include "dtmf.h"
#include "frequencies.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "ui/inputbox.h"
#include "ui/ui.h"
#include <stdlib.h>

void toggle_chan_scanlist(void)
{	// toggle the selected channels scanlist setting

	if (SCANNER_IsScanning())
		return;

	if(!IS_MR_CHANNEL(gTxVfo->CHANNEL_SAVE)) {
#ifdef ENABLE_SCAN_RANGES
		gScanRangeStart = gScanRangeStart ? 0 : gTxVfo->pRX->Frequency;
		gScanRangeStop = gEeprom.VfoInfo[!gEeprom.TX_VFO].freq_config_RX.Frequency;
		if(gScanRangeStart > gScanRangeStop)
			SWAP(gScanRangeStart, gScanRangeStop);
#endif
		return;
	}
	
	if (gTxVfo->SCANLIST1_PARTICIPATION ^ gTxVfo->SCANLIST2_PARTICIPATION){
		gTxVfo->SCANLIST2_PARTICIPATION = gTxVfo->SCANLIST1_PARTICIPATION;
	} else {
		gTxVfo->SCANLIST1_PARTICIPATION = !gTxVfo->SCANLIST1_PARTICIPATION;
	}

	SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);

	gVfoConfigureMode = VFO_CONFIGURE;
	gFlagResetVfos    = true;
}

static void MAIN_Key_EXIT(bool bKeyPressed, bool bKeyHeld)
{
	if (!bKeyHeld && bKeyPressed) { // exit key pressed
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

#ifdef ENABLE_DTMF_CALLING
		if (gDTMF_CallState != DTMF_CALL_STATE_NONE && gCurrentFunction != FUNCTION_TRANSMIT)
		{	// clear CALL mode being displayed
			gDTMF_CallState = DTMF_CALL_STATE_NONE;
			gUpdateDisplay  = true;
			return;
		}
#endif

#ifdef ENABLE_FMRADIO
		if (!gFmRadioMode)
#endif
		{
			if (gScanStateDir == SCAN_OFF) {
				if (gInputBoxIndex == 0)
					return;
				gInputBox[--gInputBoxIndex] = 10;

				gKeyInputCountdown = key_input_timeout_500ms;

#ifdef ENABLE_VOICE
				if (gInputBoxIndex == 0)
					gAnotherVoiceID = VOICE_ID_CANCEL;
#endif
			}
			else {
				gScanKeepResult = false;
				CHFRSCANNER_Stop();

#ifdef ENABLE_VOICE
				gAnotherVoiceID = VOICE_ID_SCANNING_STOP;
#endif
			}

			gRequestDisplayScreen = DISPLAY_MAIN;
			return;
		}

#ifdef ENABLE_FMRADIO
		ACTION_FM();
#endif
		return;
	}

	if (bKeyHeld && bKeyPressed) { // exit key held down
		if (gInputBoxIndex > 0 || gDTMF_InputBox_Index > 0 || gDTMF_InputMode)
		{	// cancel key input mode (channel/frequency entry)
			gDTMF_InputMode       = false;
			gDTMF_InputBox_Index  = 0;
			memset(gDTMF_String, 0, sizeof(gDTMF_String));
			gInputBoxIndex        = 0;
			gRequestDisplayScreen = DISPLAY_MAIN;
			gBeepToPlay           = BEEP_1KHZ_60MS_OPTIONAL;
		}
	}
}

static void MAIN_Key_MENU(const bool bKeyPressed, const bool bKeyHeld)
{
	if (bKeyPressed && !bKeyHeld) // menu key pressed
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

	if (bKeyHeld) { // menu key held down (long press)
		if (bKeyPressed) { // long press MENU key

			gWasFKeyPressed = false;

			if (gScreenToDisplay == DISPLAY_MAIN) {
				if (gInputBoxIndex > 0) { // delete any inputted chars
					gInputBoxIndex        = 0;
					gRequestDisplayScreen = DISPLAY_MAIN;
				}

				gWasFKeyPressed = false;
				gUpdateStatus   = true;

				ACTION_Handle(KEY_MENU, bKeyPressed, bKeyHeld);
			}
		}

		return;
	}

	if (!bKeyPressed && !gDTMF_InputMode) { // menu key released
		const bool bFlag = !gInputBoxIndex;
		gInputBoxIndex   = 0;

		if (bFlag) {
			if (gScanStateDir != SCAN_OFF) {
				CHFRSCANNER_Stop();
				return;
			}

			gFlagRefreshSetting = true;
			gRequestDisplayScreen = DISPLAY_MENU;
			#ifdef ENABLE_VOICE
				gAnotherVoiceID   = VOICE_ID_MENU;
			#endif
		}
		else {
			gRequestDisplayScreen = DISPLAY_MAIN;
		}
	}
}

void MAIN_ProcessKeys(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	if (gAusCbMode) {
		if (Key == KEY_UP || Key == KEY_DOWN) {
			if (bKeyPressed) AUSCB_Step(Key == KEY_UP ? 1 : -1);
			return;
		}
		if (Key == KEY_STAR) {
			if (!bKeyPressed && !bKeyHeld) ACTION_Scan(false);
			return;
		}
		if (Key != KEY_MENU && Key != KEY_EXIT && Key != KEY_PTT && Key != KEY_F)
			return;
	}
    switch (Key) {
        case KEY_MENU: MAIN_Key_MENU(bKeyPressed,bKeyHeld); break;
        case KEY_EXIT: MAIN_Key_EXIT(bKeyPressed,bKeyHeld); break;
        case KEY_F: GENERIC_Key_F(bKeyPressed,bKeyHeld); break;
        case KEY_PTT: GENERIC_Key_PTT(bKeyPressed); break;
        default: break;
    }
}
