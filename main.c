//
//  main.c
//  glider2json — command line Glider 4.0 house-file to JSON converter
//
//  Ported from the original macOS app's main.m / AppDelegate.m.
//  The actual parsing logic (HouseConvert.c) is unchanged.
//

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "HouseConvert.h"


static const char *kOutputExtension = ".house4";
static const char *kDefaultOutputBaseName = "house";


static void printUsage (const char *progName) {
	fprintf (stderr,
		"Usage: %s [-o output] house_file [house_file ...]\n"
		"\n"
		"Converts one or more Glider 4.0 house files into a single JSON house file.\n"
		"If a house is split across multiple linked files, pass them all in any\n"
		"order — they'll be stitched together using each file's internal link\n"
		"info (firstFile / nextFile), not the order given on the command line.\n"
		"\n"
		"The output file always gets a '%s' extension; it's appended\n"
		"automatically if you don't include it in -o.\n"
		"\n"
		"Options:\n"
		"  -o <path>   Path to write the output to (default: %s%s)\n"
		"  -h, --help  Show this help message\n",
		progName, kOutputExtension, kDefaultOutputBaseName, kOutputExtension);
}

//! Returns just the filename portion of a path (no directory components).
//! Does not modify the input string.
static const char *baseNameOf (const char *path) {
	const char *lastSlash = strrchr (path, '/');
	if (lastSlash != NULL) {
		return lastSlash + 1;
	}
	return path;
}

//! Returns a newly malloc'd copy of `path` guaranteed to end with
//! kOutputExtension, appending it if it isn't already there. Comparison
//! is case-insensitive so "-o Foo.HOUSE4" isn't given a second extension.
static char *pathWithHouse4Extension (const char *path) {
	size_t pathLen = strlen (path);
	size_t extLen = strlen (kOutputExtension);

	bool alreadyHasExtension = false;
	if (pathLen >= extLen) {
		alreadyHasExtension = (strcasecmp (path + (pathLen - extLen), kOutputExtension) == 0);
	}

	if (alreadyHasExtension) {
		char *result = (char *) malloc (pathLen + 1);
		strcpy (result, path);
		return result;
	}

	char *result = (char *) malloc (pathLen + extLen + 1);
	strcpy (result, path);
	strcpy (result + pathLen, kOutputExtension);
	return result;
}

//! Reads an entire file into a newly malloc'd HouseStruct-sized buffer.
//! The buffer is zero-filled first and the file's bytes are copied in,
//! so a short/odd-sized file won't leave the struct partially uninitialized
//! or (worse) cause writes into fixHouseEndianess() to run past a too-small
//! allocation, as the original AppDelegate.m code could do (it malloc'd
//! exactly the file's byte length, not sizeof(HouseStruct)).
static HouseStruct *readHouseFile (const char *path) {
	FILE *fp = fopen (path, "rb");
	if (fp == NULL) {
		fprintf (stderr, "error: couldn't open '%s': %s\n", path, strerror (errno));
		return NULL;
	}

	fseek (fp, 0, SEEK_END);
	long fileSize = ftell (fp);
	fseek (fp, 0, SEEK_SET);

	if (fileSize <= 0) {
		fprintf (stderr, "error: '%s' is empty or unreadable.\n", path);
		fclose (fp);
		return NULL;
	}

	if ((size_t) fileSize != sizeof (HouseStruct)) {
		fprintf (stderr,
			"warning: '%s' is %ld bytes, expected %zu for a Glider 4.0 house file. "
			"Attempting to read it anyway.\n",
			path, fileSize, sizeof (HouseStruct));
	}

	HouseStruct *houseData = (HouseStruct *) calloc (1, sizeof (HouseStruct));
	if (houseData == NULL) {
		fprintf (stderr, "error: out of memory reading '%s'.\n", path);
		fclose (fp);
		return NULL;
	}

	size_t bytesToRead = (size_t) fileSize < sizeof (HouseStruct) ? (size_t) fileSize : sizeof (HouseStruct);
	size_t bytesRead = fread (houseData, 1, bytesToRead, fp);
	fclose (fp);

	if (bytesRead != bytesToRead) {
		fprintf (stderr, "error: failed to read '%s' (got %zu of %zu bytes).\n", path, bytesRead, bytesToRead);
		free (houseData);
		return NULL;
	}

	fixHouseEndianess (houseData);

	return houseData;
}

int main (int argc, char *argv[]) {
	const char *outputPath = NULL;
	const char *inputPaths[MAX_HOUSES];
	int inputCount = 0;

	// Parse arguments.
	for (int i = 1; i < argc; i++) {
		if ((strcmp (argv[i], "-h") == 0) || (strcmp (argv[i], "--help") == 0)) {
			printUsage (argv[0]);
			return 0;
		} else if (strcmp (argv[i], "-o") == 0) {
			if ((i + 1) >= argc) {
				fprintf (stderr, "error: -o requires a path argument.\n");
				printUsage (argv[0]);
				return 1;
			}
			outputPath = argv[++i];
		} else {
			if (inputCount >= MAX_HOUSES) {
				fprintf (stderr, "error: too many input files (max %d).\n", MAX_HOUSES);
				return 1;
			}
			inputPaths[inputCount++] = argv[i];
		}
	}

	if (inputCount == 0) {
		fprintf (stderr, "error: no house file(s) given.\n\n");
		printUsage (argv[0]);
		return 1;
	}

	// Read and byte-swap each house file.
	HouseStruct *houseDataArray[MAX_HOUSES];
	char *houseNameArray[MAX_HOUSES];
	int houseCount = 0;

	for (int i = 0; i < inputCount; i++) {
		HouseStruct *houseData = readHouseFile (inputPaths[i]);
		if (houseData == NULL) {
			// Clean up anything already read before bailing.
			for (int c = 0; c < houseCount; c++) {
				free (houseDataArray[c]);
				free (houseNameArray[c]);
			}
			return 1;
		}

		const char *baseName = baseNameOf (inputPaths[i]);
		char *nameCopy = (char *) malloc (strlen (baseName) + 1);
		strcpy (nameCopy, baseName);

		houseDataArray[houseCount] = houseData;
		houseNameArray[houseCount] = nameCopy;
		houseCount += 1;
	}

	// Convert to JSON.
	char *houseJSON = housesToJSON (houseDataArray, houseNameArray, houseCount);

	char *finalOutputPath = pathWithHouse4Extension (outputPath != NULL ? outputPath : kDefaultOutputBaseName);

	int exitCode = 0;
	if (houseJSON == NULL) {
		fprintf (stderr, "error: failed to convert house data to JSON.\n");
		exitCode = 1;
	} else {
		FILE *outFile = fopen (finalOutputPath, "wb");
		if (outFile == NULL) {
			fprintf (stderr, "error: couldn't open '%s' for writing: %s\n", finalOutputPath, strerror (errno));
			exitCode = 1;
		} else {
			fwrite (houseJSON, 1, strlen (houseJSON), outFile);
			fclose (outFile);
			printf ("Wrote %s\n", finalOutputPath);
		}
		free (houseJSON);
	}
	free (finalOutputPath);

	// Clean up.
	for (int i = 0; i < houseCount; i++) {
		free (houseDataArray[i]);
		free (houseNameArray[i]);
	}

	return exitCode;
}
