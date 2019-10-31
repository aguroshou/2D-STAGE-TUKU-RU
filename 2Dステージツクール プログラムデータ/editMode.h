#ifndef INCLUDED_EDIT_MODE_H
#define INCLUDED_EDIT_MODE_H
#include "definitions.h"

void editModeInitialize(SaveData_t *Data);
void editModeExecute(SaveData_t *Data, const int(&key)[256]);
void editModeDraw(const SaveData_t& Data, const Images_t& images);
#endif