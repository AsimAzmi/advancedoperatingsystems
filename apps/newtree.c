
#include<rbt.h>
#include <stdlib.h>
#include <stdio.h>

struct rbNode *createNode(int data) {
  struct rbNode *newnode;
  newnode = (struct rbNode *)getmem(sizeof(struct rbNode));
  newnode->data = data;
  newnode->color = RED;
  newnode->link[0] = newnode->link[1] = NULL;
  return newnode;
}