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
    rope_set(new_nodes[i], rope_length(left), NULL, NULL, left, right);
    left->parent = new_nodes[i];
    right->parent = new_nodes[i];
  }

  // handle if length is odd by adding last node as parent node
  if (length % 2 != 0) {
    RopeNode* left = nodes[length - 1];
    new_nodes[new_length - 1] = malloc(sizeof(RopeNode));
    rope_set(new_nodes[new_length - 1], rope_length(left), NULL, NULL, left, NULL);
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
  // allocate memory for new root node
  RopeNode *root = malloc(sizeof(RopeNode));
  if (root == NULL) {
    SDL_SetError("Failed to allocate memoery for node");
    return NULL;
  }

  // set two nodes to be children of new root node
  rope_set(root, rope_length(first), NULL, NULL, first, second);
  first->parent = root;
  second->parent = root;
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

RopeNode *rope_rebuild(RopeNode *root)
{
  // return if at the end of the tree
  if (root == NULL) return NULL;

  // recursively build the left and right subtrees
  RopeNode *left = rope_rebuild(root->left);
  RopeNode *right = rope_rebuild(root->right);

  // allocate memory for new root node
  RopeNode *node = malloc(sizeof(RopeNode));
  if (node == NULL) {
    SDL_SetError("Failed to allocate memory for node");
    return NULL;
  }

  // allocate memory for new text value if it is a leaf
  uint32_t *new_text = NULL;
  if (root->value != NULL) {
    new_text = malloc(root->weight * sizeof(uint32_t));
    if (new_text == NULL) {
      SDL_SetError("Failed to allocate memory for text");
      return NULL;
    }
    memcpy(new_text, root->value, root->weight * sizeof(uint32_t));
  }

  // update properties on new nodes
  rope_set(node, root->weight, new_text, root->parent, left, right);
  if (left != NULL) left->parent = node;
  if (right != NULL) right->parent = node;

  return node;
}

RopeNode **rope_split(RopeNode *root, int index)
{
  // guard to check if there is a rope
  if (root == NULL) {
    SDL_SetError("No rope to split");
    return NULL;
  }

  // obtain information about where the split point is located
  RopeIndex idx = rope_index(root, index);
  RopeNode *prev = idx.node;

  // if split point is in the middle of a leaf, create two new leaves
  if (idx.n_idx < idx.node->weight - 1) {
    // allocate memory for left and right leaves
    RopeNode *left = malloc(sizeof(RopeNode));
    RopeNode *right = malloc(sizeof(RopeNode));
    if (left == NULL || right == NULL) {
      SDL_SetError("Failed to allocate memory for node");
      return NULL;
    }

    // calculate weights for the leaves
    int l_weight = idx.n_idx + 1;
    int r_weight = idx.node->weight - idx.n_idx - 1;

    // allocate memory for the text values for the leaves
    uint32_t *left_text = malloc(l_weight * sizeof(uint32_t));
    uint32_t *right_text = malloc(r_weight * sizeof(uint32_t));
    if (left_text == NULL || right_text == NULL) {
      SDL_SetError("Failed to allocate memory for text");
      return NULL;
    }

    // copy the split text values from the parent node to the leaves
    memcpy(left_text, idx.node->value, l_weight * sizeof(uint32_t));
    memcpy(right_text, idx.node->value+idx.n_idx+1, r_weight * sizeof(uint32_t));

    // set leaf properties
    rope_set(left, l_weight, left_text, idx.node, NULL, NULL);
    rope_set(right, r_weight, right_text, idx.node, NULL, NULL);

    // update parent node to point to the two leaves
    free(idx.node->value);
    idx.node->value = NULL;
    idx.node->left = left;
    idx.node->right = right;
    prev = idx.node->left;
  }
  
  // create state variable for previously created new node
  RopeNode *new = malloc(sizeof(RopeNode));
  if (new == NULL) {
    SDL_SetError("Failed to allocate memory for node");
    return NULL;
  }
  rope_set(new, prev->weight, prev->value, NULL, NULL, NULL);

  // create pointer that points to the current node
  RopeNode *ptr = prev->parent;

  // dynamic array to keep track of new split nodes
  RopeNode **nodes = NULL;

  // split nodes while the pointer is not at the head of the tree
  while (ptr != NULL) {
    // allocate memory for new node
    RopeNode *node = malloc(sizeof(RopeNode));
    if (node == NULL) {
      SDL_SetError("Failed to allocate node");
      return NULL;
    }

    // if came from the right, create a new node with same info
    if (ptr->right == prev) {
      // make new nodes for left subtree
      RopeNode *new_left = rope_rebuild(ptr->left);
      if (new_left == NULL) return NULL;
      rope_set(node, rope_length(new_left), NULL, NULL, new_left, new);
    }

    // otherwise if there is another tree to the right, split it off
    else if (ptr->right != NULL) {
      rope_set(node, rope_length(new), NULL, NULL, new, NULL);
      RopeNode *new_right = rope_rebuild(ptr->right);
      if (new_right == NULL) return NULL;
      new_right->parent = NULL;
      arrput(nodes, new_right);
    }

    // otherwise, just create a new node with same info
    else {
      rope_set(node, rope_length(new), NULL, NULL, new, NULL);
    }

    // update state
    new->parent = node;
    new = node;
    prev = ptr;
    ptr = ptr->parent;
  }

  // allocate memory for the two root nodes
  RopeNode **roots = malloc(2 * sizeof(RopeNode*));

  // if no split nodes, exit early
  if (arrlen(nodes) == 0) {
    roots[0] = new;
    roots[1] = NULL;
    return roots;
  }

  // if only one split node, set second root node to that and return
  if (arrlen(nodes) == 1) {
    roots[0] = new;
    roots[1] = nodes[0];
    return roots;
  }

  // concat all of the split nodes into another tree
  RopeNode *right = nodes[0];
  for (int i = 1; i < arrlen(nodes); i++) {
    right = rope_concat(right, nodes[i]);
    if (right == NULL) return NULL;
  }

  // set roots and return
  roots[0] = new;
  roots[1] = right;
  return roots;
}

