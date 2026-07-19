//
//  HouseConvert.h
//

#ifndef HouseConvert_h
#define HouseConvert_h


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#define MAX_HOUSES 10


#pragma pack(1)
typedef struct {
	int16_t objectIs;
	int16_t top;
	int16_t left;
	int16_t bottom;
	int16_t right;
	int16_t amount;
	int16_t extra;
	uint8_t isOn;
	uint8_t padding;
} ObjectStruct;

typedef struct {
	char roomName[26];
	int16_t numberOObjects;
	int16_t backPictID;
	int16_t tileOrder[8];
	uint8_t leftOpen;
	uint8_t rightOpen;
	int16_t animateKind;
	int16_t animateNumber;
	int32_t animateDelay;
	int16_t conditionCode;
	ObjectStruct theObjects[16];
} RoomStruct;

typedef struct {
	int16_t version;
	int16_t numberORooms;
	int32_t timeStamp;
	int32_t hiScores[20];
	int16_t hiLevel[20];
	char hiName[20][26];
	char hiRoom[20][26];
	char pictFile[34];
	char nextFile[34];
	char firstFile[34];
	RoomStruct theRooms[40];
} HouseStruct;
#pragma pack()


void fixHouseEndianess (HouseStruct *houseData);

char *housesToJSON (HouseStruct *houseDataArray[], char *houseNameArray[], int numHouses);

#endif // HouseConvert_h
