/*
* File: mymake.c
* Author: Yosef Jacobson
* Purpose: Implementation of "make" program. Currently builds a dependency graph from an input file and traverses it post-order from an input target.
*/

#include "graphs.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static char* parseTarget(char* line);
static int isEmpty(char* line);

struct VertexNode* head = NULL;
FILE* fp;

int main(int argc, char* argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Error! This program must be called with exactly 2 arguments.\n");
		exit(1);
	}

	char* fileName = argv[1];
	fp = fopen(fileName, "r");

	if (fp == NULL) {
		// file does not exist
		perror(fileName);
		exit(1);
	}

	// process the file and build the dependency graph
	char* line = NULL;
	size_t len = 0;
	char* lastTarget = NULL;
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
				freeList(&head);
				exit(1);
			}

			// replacing the leading tab with two spaces as per spec
			char* truncLine = line + 1;
			char* formattedCmd = malloc(strlen(truncLine) + 3);
			strcpy(formattedCmd, "  ");
			strcat(formattedCmd, truncLine);

			addCmd(lastTarget, formattedCmd, head);

			free(formattedCmd);
		}

		else {
			// line is a rule
			char* deps = parseTarget(line);
			char target[65], buff[2];

			if (sscanf(line, "%64s%1s", target, buff) != 1) {
				fprintf(stderr, "Error! Illegal target: %s", line);
				fclose(fp);
				free(line);
				freeList(&head);
				exit(1);
			}

			// add the target
			if (addVertex(target, 1, head, &head) == -1) {
				// fatal error condition
				freeList(&head);
				fclose(fp);
				free(line);
				exit(1);
			}

			lastTarget = target;	// lastTarget is used to link any following commands to the proper target in later iterations

			// add all dependencies
			char currDep[65];
			int charCount = 0;
			while (sscanf(deps, "%64s%n", currDep, &charCount) > 0) {
				if (addVertex(currDep, 0, head, &head) == -1) {
					// fatal error condition
					freeList(&head);
					fclose(fp);
					free(line);
					exit(1);
				}
				addEdge(target, currDep, head);
				deps += charCount;
			}
		}
	}
	
	fclose(fp);

	free(line);

	unmarkNodes(head);
	postOrder(argv[2], head);

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