/*
* File: mymake.c
* Author: Yosef Jacobson
* Purpose: Implementation of "make" program. Functions very similarly to make but with limited functionality.
* 
* Usage: mymake2 [-f filename] [target]
*/

#include "graphs.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static char* parseTarget(char* line);
static int isEmpty(char* line);
static void handleCommands(int argc, char* argv[], char** fileName, char** aTarget, int* defaultTarget);

struct VertexNode* head = NULL;
FILE* fp;

int main(int argc, char* argv[]) {
	char* fileName = "";
	char* aTarget = "";
	int defaultTarget = 0;

	handleCommands(argc, argv, &fileName, &aTarget, &defaultTarget);

	fp = fopen(fileName, "r");

	if (fp == NULL) {
		// file does not exist
		perror(fileName);
		free(aTarget);
		exit(1);
	}

	// process the file and build the dependency graph
	char* line = NULL;
	size_t len = 0;
	char* lastTarget = NULL;
	int targFound = 0;
	while (getline(&line, &len, fp) >= 0) {
		if (isEmpty(line)) {
			continue;
		}

		if (*line == '\t') {
			// line is a command
			if (lastTarget == NULL) {
				fprintf(stderr, "Error! Command(s) precede(s) the first target.\n");
				fclose(fp);
				free(line);
				free(aTarget);
				freeList(&head);
				exit(1);
			}
	
			char* truncLine = line + 1;	// removing tab from the command for printing later

			addCmd(lastTarget, truncLine, head);
		}

		else {
			// line is a rule
			char* deps = parseTarget(line);
			char target[65], buff[2];

			if (sscanf(line, "%64s%1s", target, buff) != 1) {
				fprintf(stderr, "Error! Illegal target: %s", line);
				fclose(fp);
				free(line);
				free(aTarget);
				freeList(&head);
				exit(1);
			}

			// add the target
			addVertex(target, 1, head, &head);

			if (!targFound && defaultTarget) {
				aTarget = strdup(target);
				targFound = 1;
			}

			lastTarget = target;	// lastTarget is used to link any following commands to the proper target in later iterations

			// add all dependencies
			char currDep[65];
			int charCount = 0;
			while (sscanf(deps, "%64s%n", currDep, &charCount) > 0) {
				addVertex(currDep, 0, head, &head);
				addEdge(target, currDep, head);
				deps += charCount;
			}
		}
	}
	
	fclose(fp);

	free(line);

	unmarkNodes(head);
	int retVal = postOrder(aTarget, head, aTarget);
	if (retVal == 0) {
		printf("%s is up to date.\n", aTarget);
	}

	free(aTarget);
	freeList(&head);

	return 0;
}

/*
* Returns 1 if line contains only whitespace, 0 otherwise
*/
static int isEmpty(char* line) {
	while (*line != '\0') {
		if (!isspace(*line)) {
			return 0;
		}
		line++;
	}
	return 1;
}

/*
* Removes the colon from line for easier processing. Also checks the line for multiple colons and gives an error message if this is the case.
*/
static char* parseTarget(char* line) {
	int colonFound = 0;
	char* deps = NULL;

	while (*line != '\0') {
		if (*line == ':') {
			if (!colonFound) {
				*line = '\0';
				deps = line + 1;
				colonFound = 1;
			}

			else if (colonFound) {
				fprintf(stderr, "Error! Multiple colons in line: \"%s\"", line);
				fclose(fp);
				free(line);
				freeList(&head);
				exit(1);
			}
		}
		line++;
	}

	return deps;
}

/*
* Processes command line arguments and sets filename, aTarget, and defaultTarget to the corresponding values.
*/
static void handleCommands(int argc, char* argv[], char** fileName, char** aTarget, int* defaultTarget) {
	if (argc < 2) {
		// no arguments
		*fileName = "myMakefile";
		*defaultTarget = 1;
	}

	else if (argc == 2) {
		// 1 argument
		if (strcmp(argv[1], "-f") == 0) {
			fprintf(stderr, "Error! -f must be followed by a file name.\n");
			exit(1);
		}

		*fileName = "myMakefile";
		*aTarget = strdup(argv[1]);
	}

	else if (argc == 3) {
		// should be a -f and aMakeFile
		if (strcmp(argv[1], "-f") != 0 || strcmp(argv[2], "-f") == 0) {
			fprintf(stderr, "Unsupported command. Usage of mymake2 is \"mymake2 [-f aMakeFile] [aTarget]\".\n");
			exit(1);
		}

		*fileName = argv[2];
		*defaultTarget = 1;
	}

	else if (argc == 4) {
		// both commands
		int i = 1, fCount = 0, mfIndex;
		for (; i < 4; i++) {
			if (strcmp(argv[i], "-f") == 0) {
				fCount++;
				mfIndex = i + 1;
			}
		}

		if (fCount != 1 || mfIndex > 3) {
			fprintf(stderr, "Error! Improper usage of \"-f\". Usage of mymake2 is \"mymake2 [-f aMakeFile] [aTarget]\".\n");
			exit(1);
		}

		*fileName = argv[mfIndex];

		int targIndex = (mfIndex == 3) ? 1 : 3;		// if aMakeFile is the third argument, aTarget must be the first; else it's the third.
		*aTarget = strdup(argv[targIndex]);
	}

	else {
		fprintf(stderr, "Error! Improper usage of mymake2. Usage of mymake2 is \"mymake2 [-f aMakeFile] [aTarget]\".\n");
		exit(1);
	}
}