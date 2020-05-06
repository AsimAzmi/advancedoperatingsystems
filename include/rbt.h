#include<xinu.h>





enum nodeColor {
  RED,
  BLACK
};

struct rbNode {

    int data;
    char color;
    struct rbNode *link[2];

};
 
struct rbNode *root;

struct rbNode* createNode(int data);
struct rbNode* insertion( int data);
void deletion (int data);
void inorderTraversal(struct rbNode *node);
void search_node(struct rbNode *node, int data);
void rbt_func(int nargs, char *args[]);
