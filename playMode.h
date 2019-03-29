#ifndef INCLUDED_PLAY_MODE_H
#define INCLUDED_PLAY_MODE_H
#include "definitions.h"

void playModeInitialize(void);
void playModeDataInput(const SaveData_t& Data);
bool play(const SaveData_t& Data , const int(&key)[256]);
void playModeDraw(const SaveData_t& Data,const Images_t& images);
#endif
