//
//  HouseConvert.c
//


#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "HouseConvert.h"


enum {
	ObjectKindTable = 1,
	ObjectKindShelf = 2,
	ObjectKindBooks = 3,
	ObjectKindCabinet = 4,
	ObjectKindExitRect = 5,
	ObjectKindObstacleRect = 6,
	ObjectKindFloorVent = 8,
	ObjectKindCeilingVent = 9,
	ObjectKindCeilingDuct = 10,
	ObjectKindCandle = 11,
	ObjectKindLeftFan = 12,
	ObjectKindRightFan = 13,
	ObjectKindClock = 16,
	ObjectKindPaper = 17,
	ObjectKindGrease = 18,
	ObjectKindBonusRect = 19,
	ObjectKindBattery = 20,
	ObjectKindRubberBands = 21,
	ObjectKindLightSwitch = 24,
	ObjectKindOutlet = 25,
	ObjectKindThermostat = 26,
	ObjectKindShredder = 27,
	ObjectKindPowerSwitch = 28,
	ObjectKindGuitar = 29,
	ObjectKindDrip = 32,
	ObjectKindToaster = 33,
	ObjectKindBall = 34,
	ObjectKindFishBowl = 35,
	ObjectKindTeaKettle = 36,
	ObjectKindWindow = 37,
	ObjectKindPainting = 40,
	ObjectKindMirror = 41,
	ObjectKindWastebasket = 42,
	ObjectKindMac = 43,
	ObjectKindUpstairs = 44,
	ObjectKindDownstairs = 45
};


//! Byte swap unsigned short
uint16_t swap_uint16( uint16_t val )
{
	return (val << 8) | (val >> 8 );
}

//! Byte swap short
int16_t swap_int16( int16_t val )
{
	return (val << 8) | ((val >> 8) & 0xFF);
}

//! Byte swap unsigned int
uint32_t swap_uint32( uint32_t val )
{
	val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF );
	return (val << 16) | (val >> 16);
}

//! Byte swap int
int32_t swap_int32( int32_t val )
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF );
	return (val << 16) | ((val >> 16) & 0xFFFF);
}

void _replaceBadEllipseChars (char *name, int badCount, int maxSize) {
	size_t startLength = strlen (name);
	size_t finalLength = startLength + (badCount * 2);
	if ((finalLength + 1) > (size_t) maxSize) {
		// No room to expand the bad char.
		return;
	}
	
	name[finalLength] = '\0';
	int readCharOffset = (int) startLength - 1;
	int writeCharOffset = (int) finalLength - 1;
	while (badCount > 0) {
		if (name[readCharOffset] == (char) 0x7E) {
			name[writeCharOffset] = '.';
			writeCharOffset--;
			name[writeCharOffset] = '.';
			writeCharOffset--;
			name[writeCharOffset] = '.';
			badCount -= 1;
		} else {
			name[writeCharOffset] = name[readCharOffset];
		}
		writeCharOffset--;
		readCharOffset--;
	}
}

void fixHouseEndianess (HouseStruct *houseData) {
	// Fix endianess:
	houseData->version = swap_int16 (houseData->version);
	houseData->numberORooms = swap_int16 (houseData->numberORooms);
	houseData->timeStamp = swap_int32 (houseData->timeStamp);
	for (int i = 0; i < 20; i++) {
		houseData->hiScores[i] = swap_int32 (houseData->hiScores[i]);
	}
	for (int i = 0; i < 20; i++) {
		houseData->hiLevel[i] = swap_int16 (houseData->hiLevel[i]);
	}
	for (int i = 0; i < 20; i++) {
		int count = houseData->hiName[i][0];
		for (int c = 0; c < count; c++) {
			houseData->hiName[i][c] = houseData->hiName[i][c + 1];
		}
		houseData->hiName[i][count] = '\0';
	}
	for (int i = 0; i < 20; i++) {
		// Pascal string to C string for roomName.
		int badCharCount = 0;
		int count = houseData->hiRoom[i][0];
		for (int c = 0; c < count; c++) {
			houseData->hiRoom[i][c] = houseData->hiRoom[i][c + 1];
			if (houseData->hiRoom[i][c] == (char) 0xC9) {	// 0xC9 was the elipsis character on early Mac Toolbox
				houseData->hiRoom[i][c] = (char) 0x7E;
				badCharCount += 1;
			}
		}
		houseData->hiRoom[i][count] = '\0';
		if (badCharCount > 0) {
			_replaceBadEllipseChars (houseData->hiRoom[i], badCharCount, 26);
		}
	}
	
	int count = houseData->pictFile[0];
	for (int c = 0; c < count; c++) {
		houseData->pictFile[c] = houseData->pictFile[c + 1];
	}
	houseData->pictFile[count] = '\0';
	
	count = houseData->nextFile[0];
	for (int c = 0; c < count; c++) {
		houseData->nextFile[c] = houseData->nextFile[c + 1];
	}
	houseData->nextFile[count] = '\0';
	
	count = houseData->firstFile[0];
	for (int c = 0; c < count; c++) {
		houseData->firstFile[c] = houseData->firstFile[c + 1];
	}
	houseData->firstFile[count] = '\0';
	
	// Fix up rooms.
	for (int r = 0; r < houseData->numberORooms; r++) {
		// Pascal string to C string for roomName.
		int badCharCount = 0;
		count = houseData->theRooms[r].roomName[0];
		for (int c = 0; c < count; c++) {
			houseData->theRooms[r].roomName[c] = houseData->theRooms[r].roomName[c + 1];
			if (houseData->theRooms[r].roomName[c] == (char) 0xC9) {	// 0xC9 was the elipsis character on early Mac Toolbox
				houseData->theRooms[r].roomName[c] = (char) 0x7E;
				badCharCount += 1;
			}
		}
		houseData->theRooms[r].roomName[count] = '\0';
		if (badCharCount > 0) {
			_replaceBadEllipseChars (houseData->theRooms[r].roomName, badCharCount, 26);
		}

		houseData->theRooms[r].numberOObjects = swap_int16 (houseData->theRooms[r].numberOObjects);
		
		houseData->theRooms[r].backPictID = swap_int16 (houseData->theRooms[r].backPictID);

		for (int i = 0; i < 8; i++) {
			houseData->theRooms[r].tileOrder[i] = swap_int16 (houseData->theRooms[r].tileOrder[i]);
		}
		
		houseData->theRooms[r].animateKind = swap_int16 (houseData->theRooms[r].animateKind);
		
		houseData->theRooms[r].animateNumber = swap_int16 (houseData->theRooms[r].animateNumber);
		
		houseData->theRooms[r].animateDelay = swap_int32 (houseData->theRooms[r].animateDelay);
		
		houseData->theRooms[r].conditionCode = swap_int16 (houseData->theRooms[r].conditionCode);
		
		for (int o = 0; o < houseData->theRooms[r].numberOObjects; o++) {
			houseData->theRooms[r].theObjects[o].objectIs = swap_int16 (houseData->theRooms[r].theObjects[o].objectIs);
			houseData->theRooms[r].theObjects[o].top = swap_int16 (houseData->theRooms[r].theObjects[o].top);
			houseData->theRooms[r].theObjects[o].left = swap_int16 (houseData->theRooms[r].theObjects[o].left);
			houseData->theRooms[r].theObjects[o].bottom = swap_int16 (houseData->theRooms[r].theObjects[o].bottom);
			houseData->theRooms[r].theObjects[o].right = swap_int16 (houseData->theRooms[r].theObjects[o].right);
			houseData->theRooms[r].theObjects[o].amount = swap_int16 (houseData->theRooms[r].theObjects[o].amount);
			houseData->theRooms[r].theObjects[o].extra = swap_int16 (houseData->theRooms[r].theObjects[o].extra);
		}
	}
}

const char *_houseBackgroundFromPictID (int pictID) {
	switch (pictID) {
		case 201:
		return "wainscoting";
		break;
		
		case 202:
		return "basement";
		break;
		
		case 203:
		return "asian";
		break;
		
		case 204:
		return "kids";
		break;
		
		case 205:
		return "unfinished";
		break;
		
		case 206:
		return "wallpaper";
		break;
		
		case 207:
		return "den";
		break;
		
		case 208:
		return "bathroom";
		break;
		
		case 209:
		return "kitchen";
		break;
		
		default:
		return "plain";
		break;
	}
}

const char *_houseAnimatedKindFromIndex (int index) {
	switch (index) {
		case 0:
		return "dart";
		break;
		
		case 1:
		return "copter";
		break;
		
		case 2:
		return "balloon";
		break;
		
		default:
		return "none";
		break;
	}
}

const char *_houseConditionFromConditionCode (int code) {
	switch (code) {
		case 1:
		return "airOff";
		break;
		
		case 2:
		return "lightsOff";
		break;
		
		default:
		return "normal";
		break;
	}
}

const char *_houseObjectKindFromIndex (int index) {
	switch (index) {
		case ObjectKindTable:
		return "table";
		break;
		
		case ObjectKindShelf:
		return "shelf";
		break;
		
		case ObjectKindBooks:
		return "books";
		break;
		
		case ObjectKindCabinet:
		return "cabinet";
		break;
		
		case ObjectKindExitRect:
		return "exitRect";
		break;
		
		case ObjectKindObstacleRect:
		return "obstacleRect";
		break;
		
		case ObjectKindFloorVent:
		return "floorVent";
		break;
		
		case ObjectKindCeilingVent:
		return "ceilingVent";
		break;
		
		case ObjectKindCeilingDuct:
		return "ceilingDuct";
		break;
		
		case ObjectKindCandle:
		return "candle";
		break;
		
		case ObjectKindLeftFan:
		return "leftFan";
		break;
		
		case ObjectKindRightFan:
		return "rightFan";
		break;
		
		case ObjectKindClock:
		return "clock";
		break;
		
		case ObjectKindPaper:
		return "paper";
		break;
		
		case ObjectKindGrease:
		return "grease";
		break;
		
		case ObjectKindBonusRect:
		return "bonusRect";
		break;
		
		case ObjectKindBattery:
		return "battery";
		break;
		
		case ObjectKindRubberBands:
		return "rubberBands";
		break;
		
		case ObjectKindLightSwitch:
		return "lightSwitch";
		break;
		
		case ObjectKindOutlet:
		return "outlet";
		break;
		
		case ObjectKindThermostat:
		return "thermostat";
		break;
		
		case ObjectKindShredder:
		return "shredder";
		break;
		
		case ObjectKindPowerSwitch:
		return "powerSwitch";
		break;
		
		case ObjectKindGuitar:
		return "guitar";
		break;
		
		case ObjectKindDrip:
		return "drip";
		break;
		
		case ObjectKindToaster:
		return "toaster";
		break;
		
		case ObjectKindBall:
		return "ball";
		break;
		
		case ObjectKindFishBowl:
		return "fishBowl";
		break;
		
		case ObjectKindTeaKettle:
		return "teaKettle";
		break;
		
		case ObjectKindWindow:
		return "window";
		break;
		
		case ObjectKindPainting:
		return "painting";
		break;
		
		case ObjectKindMirror:
		return "mirror";
		break;
		
		case ObjectKindWastebasket:
		return "wastebasket";
		break;
		
		case ObjectKindMac:
		return "mac";
		break;
		
		case ObjectKindUpstairs:
		return "upstairs";
		break;
		
		case ObjectKindDownstairs:
		return "downstairs";
		break;
		
		default:
		return "null";
		break;
	}
}

bool _houseObjectKindShouldHaveIsOnProperty (int objectIs) {
	// isOn allowed for: ceilingDuct, leftFan, rightFan, grease, shredder, window.
	return ((objectIs == ObjectKindCeilingDuct) || (objectIs == ObjectKindLeftFan) ||
			(objectIs == ObjectKindRightFan) || (objectIs == ObjectKindGrease) ||
			(objectIs == ObjectKindShredder) || (objectIs == ObjectKindWindow));
}

bool _houseAddRoomsToJSON (cJSON *roomArray, RoomStruct roomsData[40], int roomCount, int roomIndexDelta) {
	bool success = false;
	
	for (int r = 0; r < roomCount; r++) {
		RoomStruct *roomData = &(roomsData[r]);
		cJSON *tileOrder;
		int tileArray[8];
		cJSON *objects;
		
		// Create container for the room.
		cJSON *oneRoom = cJSON_CreateObject ();
		
		if (cJSON_AddStringToObject (oneRoom, "name", roomData->roomName) == NULL) {
			printf ("_houseAddRoomsToJSON(); cJSON_AddStringToObject() failed to add name.\n");
			goto bail;
		}
		if (cJSON_AddStringToObject (oneRoom, "background", _houseBackgroundFromPictID (roomData->backPictID)) == NULL) {
			printf ("_houseAddRoomsToJSON(); cJSON_AddStringToObject() failed to add background.\n");
			goto bail;
		}
		
		// Tile order.
		for (int i = 0; i < 8; i++) {
			tileArray[i] = roomData->tileOrder[i];
		}
		tileOrder = cJSON_CreateIntArray ((int *) tileArray, 8);
		if (tileOrder == NULL) {
			printf ("_houseAddRoomsToJSON(); cJSON_CreateIntArray() failed.\n");
			goto bail;
		}
		if (!cJSON_AddItemToObject (oneRoom, "tileOrder", tileOrder)) {
			printf ("_houseAddRoomsToJSON(); cJSON_AddItemToObject() failed to add tileOrder.\n");
			goto bail;
		}
		
		// Whether left and right are open.
		if (cJSON_AddBoolToObject (oneRoom, "leftOpen", roomData->leftOpen) == NULL) {
			printf ("_houseAddRoomsToJSON(); cJSON_AddBoolToObject() failed to add leftOpen.\n");
			goto bail;
		}
		if (cJSON_AddBoolToObject (oneRoom, "rightOpen", roomData->rightOpen) == NULL) {
			printf ("_houseAddRoomsToJSON(); cJSON_AddBoolToObject() failed to add rightOpen.\n");
			goto bail;
		}
		
		// Animated.
		if (roomData->animateNumber > 0) {
			if (cJSON_AddNumberToObject (oneRoom, "animatedCount", roomData->animateNumber) == NULL) {
				goto bail;
			}
			if (roomData->animateKind >= 0) {
				if (cJSON_AddStringToObject (oneRoom, "animatedKind", _houseAnimatedKindFromIndex (roomData->animateKind)) == NULL) {
					goto bail;
				}
			}
		}
		
		if (roomData->animateDelay > 0) {
			// Converting the delay from seconds to 'ticks'. Perhaps that was an early bug.
			if (cJSON_AddNumberToObject (oneRoom, "animatedDelay", roomData->animateDelay * 60) == NULL) {
				goto bail;
			}
		}
		
		if (roomData->conditionCode > 0) {
			if (cJSON_AddStringToObject (oneRoom, "condition", _houseConditionFromConditionCode (roomData->conditionCode)) == NULL) {
				goto bail;
			}
		}
		
		// Now to encode the objects in the room.
		// Add object array.
		objects = cJSON_AddArrayToObject (oneRoom, "objects");
		if (objects == NULL) {
			goto bail;
		}
		
		for (int o = 0; o < roomData->numberOObjects; o++) {
			ObjectStruct *objectData = &(roomData->theObjects[o]);
			int bounds[4];
			cJSON *boundsArray;
			
			// Create container for the object.
			cJSON *oneObject = cJSON_CreateObject ();
			
			if (cJSON_AddStringToObject (oneObject, "kind", _houseObjectKindFromIndex (objectData->objectIs)) == NULL) {
				goto bail;
			}
			
			// Bounds.
			bounds[0] = objectData->left;
			bounds[1] = objectData->top;
			bounds[2] = objectData->right - objectData->left;
			bounds[3] = objectData->bottom - objectData->top;
			boundsArray = cJSON_CreateIntArray ((int *) bounds, 4);
			if (boundsArray == NULL) {
				goto bail;
			}
			if (!cJSON_AddItemToObject (oneObject, "bounds", boundsArray)) {
				goto bail;
			}
			
			if (objectData->amount > 0) {
				if ((objectData->objectIs == ObjectKindExitRect) || (objectData->objectIs == ObjectKindUpstairs) ||
						(objectData->objectIs == ObjectKindDownstairs)) {
					// Re-map 'amount' property to 'toRoomIndex'.
					if (cJSON_AddNumberToObject (oneObject, "toRoomIndex", objectData->amount - 1 + roomIndexDelta) == NULL) {
						goto bail;
					}
				} else if (objectData->objectIs == ObjectKindPowerSwitch) {
					// Re-map 'amount' property to 'toObjectIndex'.
					if (cJSON_AddNumberToObject (oneObject, "toObjectIndex", objectData->amount - 1) == NULL) {
						goto bail;
					}
				} else if ((objectData->objectIs == ObjectKindOutlet) || (objectData->objectIs == ObjectKindTeaKettle)) {
					// Re-map 'amount' property to 'delayTicks'.
					if (cJSON_AddNumberToObject (oneObject, "delayTicks", objectData->amount) == NULL) {
						goto bail;
					}
				} else {
					if (cJSON_AddNumberToObject (oneObject, "amount", objectData->amount) == NULL) {
						goto bail;
					}
				}
			}
			
			if (objectData->extra > 0) {
				if ((objectData->objectIs == ObjectKindDrip) || (objectData->objectIs == ObjectKindToaster) || (objectData->objectIs == ObjectKindFishBowl)) {
					if (cJSON_AddNumberToObject (oneObject, "delayTicks", objectData->extra) == NULL) {
						goto bail;
					}
				} else if (objectData->objectIs == ObjectKindCeilingDuct) {
					if (cJSON_AddNumberToObject (oneObject, "toRoomIndex", objectData->extra - 1 + roomIndexDelta) == NULL) {
						goto bail;
					}
				}
				else if (cJSON_AddNumberToObject (oneObject, "extra", objectData->extra) == NULL) {
					goto bail;
				}
			}
			
			if (_houseObjectKindShouldHaveIsOnProperty (objectData->objectIs)) {
				if (cJSON_AddBoolToObject (oneObject, "isOn", objectData->isOn) == NULL) {
					goto bail;
				}
			}
			
			// Add object to objects array.
			cJSON_AddItemToArray (objects, oneObject);
		}
		
		// Add room to rooms array.
		cJSON_AddItemToArray (roomArray, oneRoom);
	}
	
	success = true;
	
bail:
	
	return success;
}

char *housesToJSON (HouseStruct *houseDataArray[], char *houseNameArray[], int numHouses) {
	char *jsonString = NULL;
	int houseOrder[MAX_HOUSES];
	
	if (numHouses == 1) {
		houseOrder[0] = 0;
	} else {
		houseOrder[0] = -1;
		char *nextHouse = "";
		for (int i = 0; i < numHouses; i++) {
			// Looking for a house where 'firstFile' is the empty string.
			if (strlen (houseDataArray[i]->firstFile) == 0) {
				nextHouse = houseDataArray[i]->nextFile;
				houseOrder[0] = i;
				break;
			}
		}
		
		if (houseOrder[0] == -1) {
			// Failed to find the first house using the previous method.
			// Instead, just find a house with 'firstFile' and then loop over the houseNameArray to find the index of it.
			for (int i = 0; i < numHouses; i++) {
				if (strlen (houseDataArray[i]->firstFile) > 0) {
					for (int n = 0; n < numHouses; n++) {
						if (strcmp (houseNameArray[n], houseDataArray[i]->firstFile) == 0) {
							houseOrder[0] = n;
							break;
						}
					}
					break;
				}
			}
		}
		
		if (houseOrder[0] > -1) {
			// Having found the first house file, finish ordering the remaining parts.
			int orderIndex = 1;
			while (strlen (nextHouse) > 0) {
				for (int i = 0; i < numHouses; i++) {
					if (strcmp (houseNameArray[i], nextHouse) == 0) {
						houseOrder[orderIndex] = i;
						nextHouse = houseDataArray[i]->nextFile;
						orderIndex += 1;
						break;
					}
				}
			}
		} else {
			printf ("housesToJSON(); error: failed to find the first house file.\n");
			return NULL;
		}
	}
	
	// Create (root) container for the house.
	cJSON *jsonHouse = cJSON_CreateObject ();
	cJSON *rooms = NULL;
	
	// Add top-level fields.
	if (cJSON_AddNumberToObject (jsonHouse, "version", houseDataArray[0]->version) == NULL) {
		goto bail;
	}
	if (cJSON_AddStringToObject (jsonHouse, "title", houseNameArray[0]) == NULL) {
		goto bail;
	}
	if (cJSON_AddStringToObject (jsonHouse, "author", "") == NULL) {
		goto bail;
	}
	if (cJSON_AddStringToObject (jsonHouse, "description", "") == NULL) {
		goto bail;
	}
	if (cJSON_AddStringToObject (jsonHouse, "converted", "1.0") == NULL) {
		goto bail;
	}
	
	// Add room array.
	rooms = cJSON_AddArrayToObject (jsonHouse, "rooms");
	if (rooms == NULL) {
		goto bail;
	}
	
	int cumulativeRoomCount = 0;
	for (int i = 0; i < numHouses; i++) {
		int houseIndex = houseOrder[i];
		if (!_houseAddRoomsToJSON (rooms, houseDataArray[houseIndex]->theRooms, houseDataArray[houseIndex]->numberORooms, cumulativeRoomCount)) {
			goto bail;
		}
		cumulativeRoomCount += houseDataArray[houseIndex]->numberORooms;
		printf ("JSON'ed rooms from house %s\n", houseNameArray[houseIndex]);
	}
	
	// Get JSON.
	jsonString = cJSON_Print (jsonHouse);
	
bail:
	
	cJSON_Delete (jsonHouse);
	
	return jsonString;
}
