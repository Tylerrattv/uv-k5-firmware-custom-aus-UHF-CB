#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "app/auscb.h"
#include "app/chFrScanner.h"
#include "app/dtmf.h"
#include "driver/bk4819.h"
#include "functions.h"
#include "misc.h"
#include "settings.h"
#include "ui/inputbox.h"

// Exercise the real CB module with radio/EEPROM hardware replaced by stubs.
EEPROM_Config_t gEeprom;
VFO_Info_t *gTxVfo, *gRxVfo, *gCurrentVfo;
FUNCTION_Type_t gCurrentFunction;
int8_t gScanStateDir;
uint8_t gInputBoxIndex, gRequestSaveChannel, gUpdateStatus, gDTMF_PreviousIndex;
bool gWasFKeyPressed, gUpdateDisplay;
#ifdef ENABLE_DTMF_CALLING
DTMF_ReplyState_t gDTMF_ReplyState;
void DTMF_clear_RX(void) {}
#endif
static unsigned int setupCount, stopCount, dtmfClearCount;
void DTMF_clear_input_box(void) { ++dtmfClearCount; }
void CHFRSCANNER_Stop(void) { ++stopCount; gScanStateDir = SCAN_OFF; }
void RADIO_ConfigureSquelchAndOutputPower(VFO_Info_t *vfo) { (void)vfo; }
void RADIO_SelectVfos(void)
{
    gEeprom.RX_VFO = gEeprom.CROSS_BAND_RX_TX ? !gEeprom.TX_VFO : gEeprom.TX_VFO;
    gTxVfo = &gEeprom.VfoInfo[gEeprom.TX_VFO];
    gRxVfo = &gEeprom.VfoInfo[gEeprom.RX_VFO];
    gCurrentVfo = gTxVfo;
}
void RADIO_SetupRegisters(bool foreground)
{
    assert(foreground);
    ++setupCount;
    gCurrentFunction = FUNCTION_FOREGROUND;
}
void RADIO_InitInfo(VFO_Info_t *vfo, uint8_t channel, uint32_t frequency)
{
    memset(vfo, 0, sizeof(*vfo));
    vfo->CHANNEL_SAVE = channel;
    vfo->freq_config_RX.Frequency = frequency;
    vfo->freq_config_TX.Frequency = frequency;
    vfo->pRX = &vfo->freq_config_RX;
    vfo->pTX = &vfo->freq_config_TX;
}
int main(void)
{
    gEeprom.TX_VFO = 1;
    gEeprom.DUAL_WATCH = 2;
    gEeprom.CROSS_BAND_RX_TX = 1;
    gEeprom.ROGER = 2;
    gEeprom.SQUELCH_LEVEL = 4;
#ifdef ENABLE_VOX
    gEeprom.VOX_SWITCH = true;
#endif
    for (unsigned i = 0; i < 2; ++i) {
        gEeprom.ScreenChannel[i] = 12 + i;
        RADIO_InitInfo(&gEeprom.VfoInfo[i], 12 + i, 14500000 + i * 10000);
        strcpy(gEeprom.VfoInfo[i].Name, "ORIGINAL");
    }
    gEeprom.VfoInfo[0].FrequencyReverse = true;
    gEeprom.VfoInfo[0].pRX = &gEeprom.VfoInfo[0].freq_config_TX;
    gEeprom.VfoInfo[0].pTX = &gEeprom.VfoInfo[0].freq_config_RX;
    VFO_Info_t original[2];
    memcpy(original, gEeprom.VfoInfo, sizeof(original));
    RADIO_SelectVfos();
    gScanStateDir = SCAN_FWD;
    gWasFKeyPressed = true;
    gDTMF_PreviousIndex = 3;
    gRequestSaveChannel = 1;
    AUSCB_SetMode(true);
    assert(gAusCbMode && stopCount == 1 && dtmfClearCount == 1);
    assert(!gWasFKeyPressed && !gDTMF_PreviousIndex && !gRequestSaveChannel);
    assert(!gEeprom.DUAL_WATCH && !gEeprom.CROSS_BAND_RX_TX && !gEeprom.ROGER);
#ifdef ENABLE_VOX
    assert(!gEeprom.VOX_SWITCH);
#endif
    assert(gTxVfo == &gEeprom.VfoInfo[1] && gRxVfo == gTxVfo);
    gAusCbDuplex = true;
    for (uint8_t ch = 1; ch <= 80; ++ch) {
        AUSCB_SelectChannel(ch);
        assert(gTxVfo->pRX->Frequency == AUSCB_Frequency(ch));
        assert(gTxVfo->pTX->Frequency == AUSCB_TxFrequency(ch, true));
        assert(gTxVfo->CHANNEL_BANDWIDTH == BK4819_FILTER_BW_NARROW);
        assert(AUSCB_TxAllowed(gTxVfo->pTX->Frequency) == AUSCB_VoiceChannel(ch));
        assert(!AUSCB_TxAllowed(gTxVfo->pTX->Frequency + 1));
    }
    AUSCB_Step(1);
    assert(gAusCbChannel == 1);
    AUSCB_Step(-1);
    assert(gAusCbChannel == 80);
    AUSCB_SelectChannel(0);
    AUSCB_SelectChannel(81);
    assert(gAusCbChannel == 80);
    gTxVfo->CHANNEL_BANDWIDTH = BK4819_FILTER_BW_WIDE;
    assert(!AUSCB_TxAllowed(gTxVfo->pTX->Frequency));
    AUSCB_SelectChannel(40);
    gCurrentFunction = FUNCTION_TRANSMIT;
    unsigned int before = setupCount;
    AUSCB_SelectChannel(1);
    AUSCB_SetMode(false);
    assert(gAusCbMode && gAusCbChannel == 40 && setupCount == before);
    gCurrentFunction = FUNCTION_FOREGROUND;
    gEeprom.SQUELCH_LEVEL = 8;
    AUSCB_SetMode(false);
    assert(!gAusCbMode);
    assert(memcmp(original, gEeprom.VfoInfo, sizeof(original)) == 0);
    assert(gEeprom.ScreenChannel[0] == 12 && gEeprom.ScreenChannel[1] == 13);
    assert(gEeprom.DUAL_WATCH == 2 && gEeprom.CROSS_BAND_RX_TX == 1);
    assert(gEeprom.ROGER == 2 && gEeprom.SQUELCH_LEVEL == 4);
#ifdef ENABLE_VOX
    assert(gEeprom.VOX_SWITCH);
#endif
    AUSCB_SetMode(true);
    AUSCB_SetMode(false);
    assert(memcmp(original, gEeprom.VfoInfo, sizeof(original)) == 0);
    puts("CB mode: state restoration, channel/duplex selection, TX restrictions and TX retune guard passed");
    return 0;
}
