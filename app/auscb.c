#include <string.h>
#include "app/auscb.h"
#include "app/chFrScanner.h"
#include "app/dtmf.h"
#include "driver/bk4819.h"
#include "functions.h"
#include "misc.h"
#include "settings.h"
#include "ui/inputbox.h"

bool gAusCbMode;
bool gAusCbDuplex;
uint8_t gAusCbChannel = 1;
static VFO_Info_t previousVfo[2];
static uint8_t previousScreen[2];
static uint8_t previousDualWatch, previousCrossBand, previousRoger, previousSquelch;
#ifdef ENABLE_VOX
static bool previousVox;
#endif

void AUSCB_Apply(VFO_Info_t *vfo)
{
    vfo->CHANNEL_SAVE = FREQ_CHANNEL_FIRST + (gAircraftMode ? BAND2_108MHz : BAND7_470MHz);
    vfo->Band = gAircraftMode ? BAND2_108MHz : BAND7_470MHz;
    vfo->Modulation = gAircraftMode ? MODULATION_AM : MODULATION_FM;
    vfo->CHANNEL_BANDWIDTH = gAircraftMode ? BK4819_FILTER_BW_WIDE : BK4819_FILTER_BW_NARROW;
    vfo->STEP_SETTING = STEP_12_5kHz;
    vfo->StepFrequency = 1250;
    vfo->FrequencyReverse = false;
    vfo->TX_OFFSET_FREQUENCY_DIRECTION = TX_OFFSET_FREQUENCY_DIRECTION_OFF;
    vfo->TX_OFFSET_FREQUENCY = 0;
    vfo->SCRAMBLING_TYPE = 0;
    vfo->Compander = 0;
    vfo->DTMF_PTT_ID_TX_MODE = 0;
#ifdef ENABLE_DTMF_CALLING
    vfo->DTMF_DECODING_ENABLE = false;
#endif
    vfo->freq_config_RX.Frequency = gAircraftMode ? AIR_GetChannel()->frequency : AUSCB_Frequency(gAusCbChannel);
    vfo->freq_config_TX.Frequency = gAircraftMode ? vfo->freq_config_RX.Frequency : AUSCB_TxFrequency(gAusCbChannel, gAusCbDuplex);
    vfo->freq_config_RX.CodeType = CODE_TYPE_OFF;
    vfo->freq_config_TX.CodeType = CODE_TYPE_OFF;
    vfo->pRX = &vfo->freq_config_RX;
    vfo->pTX = &vfo->freq_config_TX;
    RADIO_ConfigureSquelchAndOutputPower(vfo);
}

void AUSCB_SelectChannel(uint8_t channel)
{
    if (channel < 1 || channel > 80 || gCurrentFunction == FUNCTION_TRANSMIT) return;
    gAusCbChannel = channel;
    if (!gAusCbMode) return;
    AUSCB_Apply(&gEeprom.VfoInfo[0]);
    AUSCB_Apply(&gEeprom.VfoInfo[1]);
    RADIO_SelectVfos();
    RADIO_SetupRegisters(true);
    gUpdateDisplay = true;
    gUpdateStatus = true;
}

void AUSCB_Step(int8_t direction)
{
    if (gAircraftMode) { AIR_Step(direction); return; }
    int channel = gAusCbChannel + (direction > 0 ? 1 : -1);
    AUSCB_SelectChannel(channel > 80 ? 1 : channel < 1 ? 80 : channel);
}

bool AUSCB_TxAllowed(uint32_t frequency)
{
    return !gAircraftMode && AUSCB_VoiceChannel(gAusCbChannel) &&
           frequency == AUSCB_TxFrequency(gAusCbChannel, gAusCbDuplex) &&
           gCurrentVfo->Modulation == MODULATION_FM &&
           gCurrentVfo->CHANNEL_BANDWIDTH == BK4819_FILTER_BW_NARROW;
}

void AUSCB_SetMode(bool enabled)
{
    if (enabled == gAusCbMode || gCurrentFunction == FUNCTION_TRANSMIT) return;
    if (gScanStateDir != SCAN_OFF) CHFRSCANNER_Stop();
    gInputBoxIndex = 0;
    gWasFKeyPressed = false;
    DTMF_clear_input_box();
    gDTMF_PreviousIndex = 0;
#ifdef ENABLE_DTMF_CALLING
    DTMF_clear_RX();
    gDTMF_ReplyState = DTMF_REPLY_NONE;
#endif
    gRequestSaveChannel = 0;
    if (enabled) {
        memcpy(previousVfo, gEeprom.VfoInfo, sizeof(previousVfo));
        memcpy(previousScreen, gEeprom.ScreenChannel, sizeof(previousScreen));
        previousDualWatch = gEeprom.DUAL_WATCH;
        previousCrossBand = gEeprom.CROSS_BAND_RX_TX;
        previousRoger = gEeprom.ROGER;
        previousSquelch = gEeprom.SQUELCH_LEVEL;
#ifdef ENABLE_VOX
        previousVox = gEeprom.VOX_SWITCH;
        gEeprom.VOX_SWITCH = false;
#endif
        gEeprom.DUAL_WATCH = DUAL_WATCH_OFF;
        gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_OFF;
        gEeprom.ROGER = 0;
        gSetting_live_DTMF_decoder = false;
#ifdef ENABLE_AM_FIX
        gSetting_AM_fix = true;
#endif
        gAusCbMode = true;
        for (unsigned int i = 0; i < 2; ++i) {
            gEeprom.ScreenChannel[i] = FREQ_CHANNEL_FIRST + BAND7_470MHz;
            RADIO_InitInfo(&gEeprom.VfoInfo[i], gEeprom.ScreenChannel[i], AUSCB_Frequency(gAusCbChannel));
        }
        AUSCB_SelectChannel(gAusCbChannel);
    } else {
        gAusCbMode = false;
        gAircraftMode = false;
        memcpy(gEeprom.VfoInfo, previousVfo, sizeof(previousVfo));
        memcpy(gEeprom.ScreenChannel, previousScreen, sizeof(previousScreen));
        gEeprom.DUAL_WATCH = previousDualWatch;
        gEeprom.CROSS_BAND_RX_TX = previousCrossBand;
        gEeprom.ROGER = previousRoger;
        gEeprom.SQUELCH_LEVEL = previousSquelch;
#ifdef ENABLE_VOX
        gEeprom.VOX_SWITCH = previousVox;
#endif
        RADIO_SelectVfos();
        RADIO_SetupRegisters(true);
    }
    gUpdateDisplay = true;
    gUpdateStatus = true;
}
