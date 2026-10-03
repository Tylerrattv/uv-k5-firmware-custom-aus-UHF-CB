#include "app/aircraft.h"
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
    for (uint16_t i=0;i<gAirportsCount;i++)
        if (gAirports[i].region==gAirRegion && index--==0) return &gAirports[i];
    return &gAirports[0];
}
const AirChannel *AIR_GetChannel(void) { return &gAirChannels[AIR_GetAirport(gAirAirport)->first+gAirChannel]; }
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
    for (uint16_t i=0;i<a->count;i++) if (gAirChannels[a->first+i].frequency==frequency) { AIR_SelectChannel(i);return; }
    AIR_SelectChannel(0);
}
void AIR_SetMode(bool enabled) {
    if (gCurrentFunction==FUNCTION_TRANSMIT) return;
    if (gScanStateDir!=SCAN_OFF) CHFRSCANNER_Stop();
    AUSCB_SetMode(true);
    gAircraftMode=enabled;
    AUSCB_SelectChannel(gAusCbChannel);
}
