#ifndef APP_AUSCB_PLAN_H
#define APP_AUSCB_PLAN_H
#include <stdbool.h>
#include <stdint.h>

// Firmware frequency units are 10 Hz. Channels 41-80 interleave 1-40.
static inline uint32_t AUSCB_Frequency(uint8_t channel)
{
    if (channel < 1 || channel > 80) return 0;
    return channel <= 40 ? 47642500u + (channel - 1u) * 2500u
                         : 47643750u + (channel - 41u) * 2500u;
}
static inline bool AUSCB_IsRepeater(uint8_t channel)
{
    return (channel >= 1 && channel <= 8) || (channel >= 41 && channel <= 48);
}
static inline uint32_t AUSCB_TxFrequency(uint8_t channel, bool duplex)
{
    uint32_t frequency = AUSCB_Frequency(channel);
    return frequency && duplex && AUSCB_IsRepeater(channel) ? frequency + 75000u : frequency;
}
static inline bool AUSCB_VoiceChannel(uint8_t channel)
{
    return channel >= 1 && channel <= 80 && channel != 22 && channel != 23 &&
           channel != 61 && channel != 62 && channel != 63;
}
static inline uint8_t AUSCB_ChannelFromFrequency(uint32_t frequency)
{
    for (uint8_t channel = 1; channel <= 80; ++channel)
        if (AUSCB_Frequency(channel) == frequency) return channel;
    return 1;
}

// Short Australian channel-use hints, max 18 characters for the LCD.
// Road/convoy labels describe customary use, not exclusive allocations.
static inline const char *AUSCB_ChannelUse(uint8_t channel)
{
    if (channel < 1 || channel > 80) return "INVALID CHANNEL";
    if (channel == 5 || channel == 35) return "EMERGENCY ONLY";
    if (channel == 22 || channel == 23) return "DATA ONLY";
    if (channel >= 61 && channel <= 63) return "RESERVED";
    if (AUSCB_IsRepeater(channel)) return "REPEATER OUTPUT";
    if ((channel >= 31 && channel <= 38) || (channel >= 71 && channel <= 78))
        return "REPEATER INPUT";
    switch (channel) {
        case 10: return "4WD / CONVOY";
        case 11: return "CALLING";
        case 18: return "CARAVANS / CAMPERS";
        case 29: return "PACIFIC/BRUCE HWY";
        case 40: return "ROAD / TRUCKS";
        default: return "GENERAL USE";
    }
}
#endif
