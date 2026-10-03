#ifndef APP_AUSCB_H
#define APP_AUSCB_H
#include "radio.h"
#include "app/auscb_plan.h"
extern bool gAusCbMode;
extern bool gAusCbDuplex;
extern uint8_t gAusCbChannel;
void AUSCB_SetMode(bool enabled);
void AUSCB_SelectChannel(uint8_t channel);
void AUSCB_Step(int8_t direction);
void AUSCB_Apply(VFO_Info_t *vfo);
bool AUSCB_TxAllowed(uint32_t frequency);
#endif
