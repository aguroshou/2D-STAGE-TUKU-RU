#ifndef SAVE_AND_LOAD_H
#define SAVE_AND_LOAD_H
void save(const SaveData_t& Data,bool isFullScreen);
void load(SaveData_t* Data, bool isFullScreen);
void imagesLoad(Images_t* images);
#endif
