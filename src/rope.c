#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL_error.h>

#include "rope.h"
#include "stb_ds.h"

void rope_set(RopeNode *node, int w, uint32_t *val, RopeNode *par, RopeNode *l,
              RopeNode *r)
{
  node->weight = w;
  node->value = val;
  node->parent = par;
  node->left = l;
  node->right = r;
}

RopeNode *rope_merge(RopeNode **nodes, int length)
{
  // return if array only consists of one node, the root node
  if (length == 1) {
    RopeNode *root = nodes[0];
    free(nodes);
    return root;
  }

  // calculate length of next level of nodes and allocate memory for array
  int new_length = (length + 1)/ 2;
  RopeNode **new_nodes = malloc(new_length * sizeof(RopeNode*));

  // iterate through pairs of nodes, creating parent nodes
  for (int i = 0; i < length / 2; i++) {
    RopeNode* left = nodes[2 * i];
    RopeNode* right = nodes[2 * i + 1];
    new_nodes[i] = malloc(sizeof(RopeNode));
    rope_set(new_nodes[i], left->weight, NULL, NULL, left, right);
    left->parent = new_nodes[i];
    right->parent = new_nodes[i];
  }

  // handle if length is odd by adding last node as parent node
  if (length % 2 != 0) {
    RopeNode* left = nodes[length - 1];
    new_nodes[new_length - 1] = malloc(sizeof(RopeNode));
    rope_set(new_nodes[new_length - 1], left->weight, NULL, NULL, left, NULL);
    left->parent = new_nodes[new_length - 1];
  }

  free(nodes);
  return rope_merge(new_nodes, new_length);
}

RopeNode *rope_build(uint32_t *text, int length)
{
  // there must be text
  if (length <= 0) {
    SDL_SetError("Cannot build a rope without text");
    return NULL;
  }

  // find amount of leaves and allocate memory for them
  int new_length = (length + LEAF_WEIGHT - 1) / LEAF_WEIGHT;
  RopeNode **new_nodes = malloc(new_length * sizeof(RopeNode*));
  if (new_nodes == NULL) {
    SDL_SetError("Failed to allocate memory for nodes");
    return NULL;
  }

  // iterate through text and create new leaves
  for (int i = 0; i < new_length; i++) {
    // handle weight if there is a remainder
    int weight = LEAF_WEIGHT;
    if (i == new_length - 1 && length % LEAF_WEIGHT != 0) {
      weight = length % LEAF_WEIGHT;
    }

    // allocate new rope node
    new_nodes[i] = malloc(sizeof(RopeNode));
    if (new_nodes[i] == NULL) {
      SDL_SetError("Failed to allocate memory for node");
      return NULL;
    }

    // allocate memory for the leaf's text
    uint32_t *split_text = malloc(weight * sizeof(uint32_t));
    if (split_text == NULL) {
      SDL_SetError("Failed to allocate memory for leaf text");
      return NULL;
    }

    // copy text into node and set properties
    memcpy(split_text, &text[LEAF_WEIGHT * i], weight * sizeof(uint32_t));
    rope_set(new_nodes[i], weight, split_text, NULL, NULL, NULL);
  }

  // merge all of the leaves into a rope tree recursively
  return rope_merge(new_nodes, new_length);
}

RopeNode **rope_collect(RopeNode *root)
{
  // exit if there is no rope
  if (root == NULL) {
    SDL_SetError("Rope does not exist");
    return NULL;
  }

  // create dynamic arrays of nodes and leaves for the rope
  RopeNode **nodes = NULL;
  RopeNode **leaves = NULL;

  // traverse the rope by going down the left spine, then the right
  RopeNode *curr = root;
  while (curr != NULL) {
    // this is a node
    if (curr->left != NULL) {
      arrput(nodes, curr);
      curr = curr->left;
    }

    // this is a leaf
    else {
      arrput(leaves, curr);
      curr = NULL;
      while (curr == NULL && arrlen(nodes) > 0) {
        curr = arrlast(nodes)->right;
        arrdel(nodes, arrlen(nodes) - 1);
      }
    }
  }

  assert(arrlen(nodes) == 0);
  
  // free the dynamic array for nodes and return leaves
  arrfree(nodes);
  return leaves;
}

uint32_t *rope_text(RopeNode *root)
{
  // collect all the leaves from the rope
  RopeNode **leaves = rope_collect(root);
  if (leaves == NULL) return NULL;

  // create new dynamic array for text
  uint32_t *text = NULL;

  // iterate through leaves and add to the dynamic array
  for (int i = 0; i < arrlen(leaves); i++) {
    RopeNode *leaf = leaves[i];
    uint32_t *value = leaf->value;
    for (int j = 0; j < leaf->weight; j++) {
      arrput(text, value[j]);
    }
  }

  arrfree(leaves);
  return text;
}

void rope_free(RopeNode *root)
{
  // exit if there is no rope
  if (root == NULL) return;

  // recursively free nodes down the left and right subtrees
  rope_free(root->left);
  rope_free(root->right);

  // if it is a leaf, free the values
  if (root->value != NULL) free(root->value);

  // free the node itself
  free(root);
}

int rope_length(RopeNode *root)
{
  if (root->right == NULL) return root->weight;
  else return root->weight + rope_length(root->right);
}

int rope_height(RopeNode *root, int curr_height)
{
  if (root == NULL) return curr_height;
  curr_height++;
  int lh = rope_height(root->left, curr_height);
  int rh = rope_height(root->right, curr_height);
  return lh > rh ? lh : rh;
}

RopeNode *rope_concat(RopeNode *first, RopeNode *second)
{
  RopeNode *root = malloc(sizeof(RopeNode));
  rope_set(root, rope_length(first), NULL, NULL, first, second);
  return root;
}

RopeIndex rope_index(RopeNode *root, int index)
{
  if (index >= root->weight) return rope_index(root->right, index - root->weight);
  else if (root->left != NULL) return rope_index(root->left, index);
  else {
    RopeIndex idx = {
      .node = root,
      .c = root->value[index],
      .n_idx = index
    };
    return idx;
  }
}

