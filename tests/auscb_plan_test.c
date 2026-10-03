#include <assert.h>
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
    return 0;
}
