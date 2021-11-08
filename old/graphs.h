#ifndef GRAPHS_H
#define GRAPHS_H

struct VertexNode {
	char* name;
	int visited, isTarget;
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

int addVertex(char*, int, struct VertexNode*, struct VertexNode**);

void addEdge(char*, char*, struct VertexNode*);

void addCmd(char*, char*, struct VertexNode*);

void freeList(struct VertexNode**);

struct VertexNode* findNode(char*, struct VertexNode*);

void postOrder(char*, struct VertexNode*);

void unmarkNodes(struct VertexNode* head);


#endif