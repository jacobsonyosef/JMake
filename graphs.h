#ifndef GRAPHS_H
#define GRAPHS_H

struct VertexNode {
	char* name;
	int visited, processed, isTarget, fileExists, toBuild;
	double fileDate;
	struct EdgeNode* edgeList;
	struct CommandNode* cmdList;
	struct VertexNode* next;
};

struct EdgeNode {
	struct VertexNode* to;
	struct EdgeNode* next;
};

struct CommandNode {
	char* command;
	struct CommandNode* next;
};

void addVertex(char*, int, struct VertexNode*, struct VertexNode**);

void addEdge(char*, char*, struct VertexNode*);

void addCmd(char*, char*, struct VertexNode*);

void freeList(struct VertexNode**);

struct VertexNode* findNode(char*, struct VertexNode*);

int postOrder(char*, struct VertexNode*, char*);

void unmarkNodes(struct VertexNode* head);


#endif