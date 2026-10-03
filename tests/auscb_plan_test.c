#include <assert.h>
#include <string.h>
#include "../app/auscb_plan.h"
int main(void)
{
    assert(AUSCB_Frequency(0) == 0 && AUSCB_Frequency(81) == 0);
    assert(AUSCB_Frequency(1) == 47642500u);
    assert(AUSCB_Frequency(40) == 47740000u);
    assert(AUSCB_Frequency(41) == 47643750u);
    assert(AUSCB_Frequency(80) == 47741250u);
    for (uint8_t c = 1; c <= 80; ++c) {
        assert(AUSCB_ChannelFromFrequency(AUSCB_Frequency(c)) == c);
        assert(AUSCB_TxFrequency(c, false) == AUSCB_Frequency(c));
        if (AUSCB_IsRepeater(c)) {
            assert(AUSCB_TxFrequency(c, true) == AUSCB_Frequency(c + 30));
        } else {
            assert(AUSCB_TxFrequency(c, true) == AUSCB_Frequency(c));
        }
        for (uint8_t other = c + 1; other <= 80; ++other)
            assert(AUSCB_Frequency(c) != AUSCB_Frequency(other));
    }
    const uint8_t blocked[] = {0, 22, 23, 61, 62, 63, 81};
    for (unsigned i = 0; i < sizeof(blocked); ++i)
        assert(!AUSCB_VoiceChannel(blocked[i]));
    assert(AUSCB_VoiceChannel(5) && AUSCB_VoiceChannel(35));
    assert(AUSCB_VoiceChannel(60) && AUSCB_VoiceChannel(64) && AUSCB_VoiceChannel(80));
    for (uint8_t c = 1; c <= 80; ++c) {
        const char *label = AUSCB_ChannelUse(c);
        assert(strlen(label) > 0 && strlen(label) <= 18);
        if (c == 5 || c == 35) assert(strcmp(label, "EMERGENCY ONLY") == 0);
        else if (c == 22 || c == 23) assert(strcmp(label, "DATA ONLY") == 0);
        else if (c >= 61 && c <= 63) assert(strcmp(label, "RESERVED") == 0);
        else if (AUSCB_IsRepeater(c)) assert(strcmp(label, "REPEATER OUTPUT") == 0);
        else if ((c >= 31 && c <= 38) || (c >= 71 && c <= 78))
            assert(strcmp(label, "REPEATER INPUT") == 0);
    }
    assert(strcmp(AUSCB_ChannelUse(10), "4WD / CONVOY") == 0);
    assert(strcmp(AUSCB_ChannelUse(11), "CALLING") == 0);
    assert(strcmp(AUSCB_ChannelUse(18), "CARAVANS / CAMPERS") == 0);
    assert(strcmp(AUSCB_ChannelUse(29), "PACIFIC/BRUCE HWY") == 0);
    assert(strcmp(AUSCB_ChannelUse(40), "ROAD / TRUCKS") == 0);
    assert(strcmp(AUSCB_ChannelUse(80), "GENERAL USE") == 0);
    assert(strcmp(AUSCB_ChannelUse(0), "INVALID CHANNEL") == 0);
    assert(strcmp(AUSCB_ChannelUse(81), "INVALID CHANNEL") == 0);
    return 0;
}
