/*
* File: graphs.c
* Author: Yosef Jacobson
* Purpose: Utility class which contains many functions for building and traversing graphs.
*/

#include "graphs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
* Adds a vertex named name to the dependency graph starting at head. hp is the address of head in case head needs to be allocated.
* If the node being added is a target, isTarget will be 1; else 0.
* 
* Returns 1 if the vertex was added, 0 for a non-fatal error, and -1 for a fatal error.
*/
void addVertex(char* name, int isTarget, struct VertexNode* head, struct VertexNode** hp) {
	if (head == NULL) {
		*hp = calloc(1, sizeof(struct VertexNode));	// allocate memory for head using the passed in address
		if (*hp == NULL) {
			fprintf(stderr, "Out of memory.\n");
			exit(1);
		}
		head = *hp;

		head->name = strdup(name);
		head->isTarget = isTarget;
	}

	else {
		struct VertexNode* curr = head;

		struct VertexNode* dup = findNode(name, head);
		if (dup != NULL) {
				if (isTarget) {
					dup->isTarget = 1;	// if this node already exists but is now being added as a target, ensure the target flag is set
				}
				return;
		}

		// go to last node
		curr = head;
		while (curr->next != NULL) {
			curr = curr->next;
		}

		curr->next = calloc(1, sizeof(struct VertexNode));
		if (curr->next == NULL) {
			fprintf(stderr, "Out of memory.\n");
			exit(1);
		}
		curr->next->name = strdup(name);
		curr->next->isTarget = isTarget;
	}
}

/*
* Adds an edge from vertex name1 to vertex name2 in adjacency list starting at head.
*/
void addEdge(char* name1, char* name2, struct VertexNode* head) {
	struct VertexNode* vert1 = NULL,
		* vert2 = NULL;

	vert1 = findNode(name1, head);
	vert2 = findNode(name2, head);

	if (vert1 == NULL || vert2 == NULL) {
		fprintf(stderr, "Error! Tried adding edge between non-existent vertex(es).\n");
		return;
	}

	// if this is the first edge for this vertex, initialize edgeList
	if (vert1->edgeList == NULL) {
		vert1->edgeList = calloc(1, sizeof(struct EdgeNode));
		if (vert1->edgeList == NULL) {
			fprintf(stderr, "Out of memory.\n");
			exit(1);
		}

		vert1->edgeList->to = vert2;
	}

	else {
		struct EdgeNode* currEdge = vert1->edgeList;
		while (currEdge != NULL) {
			if (currEdge->to == vert2) {
				fprintf(stderr, "Error! Edge already exists.\n");
				return;
			}

			currEdge = currEdge->next;
		}

		// move to last edge
		currEdge = vert1->edgeList;
		while (currEdge->next != NULL) {
			currEdge = currEdge->next;
		}

		// add new edge to list
		currEdge->next = calloc(1, sizeof(struct EdgeNode));
		if (currEdge->next == NULL) {
			fprintf(stderr, "Out of memory.\n");
			exit(1);
		}

		currEdge->next->to = vert2;
	}
}

/*
* Adds the specified command to the command list of the node named name
*/
void addCmd(char* name, char* command, struct VertexNode* head) {
	struct VertexNode* target = findNode(name, head);

	if (target == NULL) {
		fprintf(stderr, "Error! Tried adding command to non-existent node.\n");
		return;
	}

	// if this is the first command for this vertex, initialize cmdlist
	if (target->cmdList == NULL) {
		target->cmdList = calloc(1, sizeof(struct CommandNode));
		if (target->cmdList == NULL) {
			fprintf(stderr, "Out of memory.\n");
			exit(1);
		}

		target->cmdList->command = strdup(command);
	}

	else {
		struct CommandNode* curr = target->cmdList;

		while (curr->next != NULL) {
			curr = curr->next;
		}

		curr->next = calloc(1, sizeof(struct CommandNode));
		if (curr->next == NULL) {
			fprintf(stderr, "Out of memory.\n");
			exit(1);
		}

		curr->next->command = strdup(command);
	}
}

/*
* Returns the node associated with name, or NULL if none is found
*/
struct VertexNode* findNode(char* name, struct VertexNode* head) {
	struct VertexNode* curr = head;

	while (curr != NULL) {
		if (strcmp(curr->name, name) == 0) {
			return curr;
		}

		curr = curr->next;
	}

	return NULL;
}

/*
* Iterates through the graph and sets all flags to default values.
*/
void unmarkNodes(struct VertexNode* head) {
	struct VertexNode* curr = head;

	while (curr != NULL) {
		curr->visited = 0;
		curr->processed = 0;
		curr->toBuild = 0;
		curr = curr->next;
	}
}

/*
* Performs a post-order traversal of head's graph from targetName, building any files necessary along the way.
* Returns 1 if any commands failed, 2 if commands were executed, and 0 otherwise.
*/
int postOrder(char* targetName, struct VertexNode* head, char* aTarget) {
	static int returnCode = 0;
	struct VertexNode* target = findNode(targetName, head);

	if (target == NULL) {
		fprintf(stderr, "Error! Target \"%s\" does not exist.\n", targetName);
		free(aTarget);
		freeList(&head);
		exit(1);
	}

	if (target->visited) {
		return returnCode;
	}

	target->visited = 1;
	struct stat* fileInfo = malloc(sizeof(struct stat));
	int fileExists = stat(targetName, fileInfo) != 0 ? 0 : 1;	// getting file details

	target->fileExists = fileExists;
	if (fileExists) {
		target->fileDate = fileInfo->st_mtime;
	}

	else {
		if (!target->isTarget) {
			fprintf(stderr, "Error! File does not exist: \"%s\"\n", targetName);
			free(fileInfo);
			free(aTarget);
			freeList(&head);
			exit(1);
		}

		target->fileDate = 0;
		target->toBuild = 1;
	}

	// visit all children in post-order
	struct EdgeNode* currEdge = target->edgeList;
	while (currEdge != NULL) {
		struct VertexNode* child = currEdge->to;

		free(fileInfo);
		postOrder(child->name, head, aTarget);
		fileInfo = malloc(sizeof(struct stat));

		if (!child->processed) {
			// cycle detected
			fprintf(stderr, "Dependency cycle found: \"%s\"\n", child->name);
		}

		else if (!target->toBuild) {
			if (!child->fileExists || (child->fileDate) > (target->fileDate)) {
				target->toBuild = 1;
			}
		}

		currEdge = currEdge->next;
	}

	// if target needs to be built, run all bash commands associated with it
	if (target->toBuild) {
		struct CommandNode* currCmd = target->cmdList;
		while (currCmd != NULL) {
			printf("%s", currCmd->command);
			int cmdRtVal = system(currCmd->command);
			if (cmdRtVal == -1 || WEXITSTATUS(cmdRtVal) != 0) {
				fprintf(stderr, "Command failed!\n");
				free(fileInfo);
				free(aTarget);
				freeList(&head);
				exit(1);
			}
			returnCode = 2;
			currCmd = currCmd->next;
		}

		fileExists = stat(targetName, fileInfo);	// update file info
		target->fileExists = fileExists;
		if (fileExists) {
			target->fileDate = fileInfo->st_mtime;
		}
	}

	free(fileInfo);
	target->processed = 1;
	return returnCode;
}

/*
* Given the head of a dependency graph, frees all memory associated with it
*/
void freeList(struct VertexNode** head) {
	struct VertexNode* curr0,
		* curr1;
	curr0 = *head;

	while (curr0 != NULL) {
		struct EdgeNode* edge0,
			* edge1;

		struct CommandNode* cmd0,
			* cmd1;

		curr1 = curr0->next;

		edge0 = curr0->edgeList;
		while (edge0 != NULL) {
			edge1 = edge0->next;
			free(edge0);
			edge0 = edge1;
		}

		cmd0 = curr0->cmdList;
		while (cmd0 != NULL) {
			cmd1 = cmd0->next;
			free(cmd0->command);
			free(cmd0);
			cmd0 = cmd1;
		}

		free(curr0->name);
		free(curr0);
		curr0 = curr1;
	}
}