#include<rbt.h>
#include <stdlib.h>
#include <stdio.h>




void search_node(struct rbNode *node, int data) {
  if (node) 
  {
  	if ( node->data == data)
  	{
  		printf("\n Element is found");
  		return;
  	}
    search_node(node->link[0],data);
    //printf("%d  ", node->data);
    search_node(node->link[1], data);
  }
  //printf("\n Element not found");
  return;
}



void inorderTraversal(struct rbNode *node) {
  if (node) 
  {
    inorderTraversal(node->link[0]);
    printf("%d  ", node->data);
    inorderTraversal(node->link[1]);
  }
  return;
}