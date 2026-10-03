#include "app/aircraft.h"
#include <string.h>
#include "app/auscb.h"
#include "app/chFrScanner.h"
#include "functions.h"
#include "misc.h"
bool gAircraftMode;
uint8_t gAirRegion;
uint16_t gAirAirport, gAirChannel;
uint16_t AIR_AirportCount(uint8_t region) {
    uint16_t count=0;
    for (uint16_t i=0;i<gAirportsCount;i++) if (gAirports[i].region==region) count++;
    return count;
}
const AirAirport *AIR_GetAirport(uint16_t index) {
    static AirAirport decoded;
    const AirAirportPacked *packed=&gAirports[0];
    for (uint16_t i=0;i<gAirportsCount;i++)
        if (gAirports[i].region==gAirRegion && index--==0) { packed=&gAirports[i];break; }
    for (unsigned i=0;i<24;i++) {
        const unsigned bit=i*6, shift=bit%8;
        unsigned value=packed->text[bit/8]>>shift;
        if (shift>2) value|=(unsigned)packed->text[bit/8+1]<<(8-shift);
        const char c=(value&63)+32;
        if (i<16) decoded.name[i]=c; else decoded.ident[i-16]=c;
    }
    decoded.name[16]=decoded.ident[8]=0;
    for (int i=15;i>=0 && decoded.name[i]==' ';i--) decoded.name[i]=0;
    for (int i=7;i>=0 && decoded.ident[i]==' ';i--) decoded.ident[i]=0;
    decoded.first=packed->first; decoded.count=packed->count; decoded.region=packed->region;
    return &decoded;
}
const AirChannel *AIR_GetChannel(void) {
    static AirChannel decoded;
    const AirChannelPacked *p=&gAirChannels[gAirChannelIndex[AIR_GetAirport(gAirAirport)->first+gAirChannel]];
    decoded.frequency=11800000u+(uint32_t)p->offset500*50u;
    decoded.service=p->service;
    return &decoded;
}
static void refresh(void) { if (gAircraftMode) AUSCB_SelectChannel(gAusCbChannel); }
void AIR_SelectRegion(uint8_t region) {
    if (region>=8 || !AIR_AirportCount(region) || gCurrentFunction==FUNCTION_TRANSMIT) return;
    if (gScanStateDir!=SCAN_OFF) CHFRSCANNER_Stop();
    gAirRegion=region;gAirAirport=0;gAirChannel=0;refresh();
}
void AIR_SelectAirport(uint16_t airport) {
    if (airport>=AIR_AirportCount(gAirRegion) || gCurrentFunction==FUNCTION_TRANSMIT) return;
    if (gScanStateDir!=SCAN_OFF) CHFRSCANNER_Stop();
    gAirAirport=airport;gAirChannel=0;refresh();
}
void AIR_SelectChannel(uint16_t channel) {
    if (channel>=AIR_GetAirport(gAirAirport)->count || gCurrentFunction==FUNCTION_TRANSMIT) return;
    gAirChannel=channel;refresh();
}
void AIR_Step(int8_t direction) {
    const uint16_t count=AIR_GetAirport(gAirAirport)->count;
    AIR_SelectChannel(direction>0 ? (gAirChannel+1)%count : (gAirChannel ? gAirChannel-1 : count-1));
}
void AIR_RestoreFrequency(uint32_t frequency) {
    const AirAirport *a=AIR_GetAirport(gAirAirport);
    for (uint16_t i=0;i<a->count;i++) if (11800000u+(uint32_t)gAirChannels[gAirChannelIndex[a->first+i]].offset500*50u==frequency) { AIR_SelectChannel(i);return; }
    AIR_SelectChannel(0);
}
void AIR_SetMode(bool enabled) {
    if (gCurrentFunction==FUNCTION_TRANSMIT) return;
    if (gScanStateDir!=SCAN_OFF) CHFRSCANNER_Stop();
    AUSCB_SetMode(true);
    gAircraftMode=enabled;
    AUSCB_SelectChannel(gAusCbChannel);
}
