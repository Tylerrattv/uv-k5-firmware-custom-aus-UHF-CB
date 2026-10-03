#ifndef APP_AIRCRAFT_H
#define APP_AIRCRAFT_H
#include <stdint.h>
#include <stdbool.h>
typedef struct { char name[17]; char ident[9]; uint16_t first; uint8_t count, region; } AirAirport;
typedef struct { uint32_t frequency; uint8_t service; } AirChannel;
extern const AirAirport gAirports[];
extern const AirChannel gAirChannels[];
extern const uint16_t gAirportsCount;
extern const char *const gAirServices[];
extern const char *const gAirRegions[8];
extern uint8_t gAirRegion;
extern uint16_t gAirAirport, gAirChannel;
extern bool gAircraftMode;
uint16_t AIR_AirportCount(uint8_t region);
const AirAirport *AIR_GetAirport(uint16_t index);
const AirChannel *AIR_GetChannel(void);
void AIR_SelectRegion(uint8_t region);
void AIR_SelectAirport(uint16_t airport);
void AIR_SelectChannel(uint16_t channel);
void AIR_Step(int8_t direction);
void AIR_RestoreFrequency(uint32_t frequency);
void AIR_SetMode(bool enabled);
#endif
