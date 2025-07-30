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
  if (new_nodes == NULL) {
    SDL_SetError("Failed to allocate memory in rope_merge");
    return NULL;
  }

  // null initialize array of new nodes
  for (int i = 0; i < new_length; i++) {
    new_nodes[i] = NULL;
  }

  // iterate through pairs of nodes, creating parent nodes
  for (int i = 0; i < length / 2; i++) {
    RopeNode* left = nodes[2 * i];
    RopeNode* right = nodes[2 * i + 1];
    new_nodes[i] = malloc(sizeof(RopeNode));
    if (new_nodes == NULL) goto cleanup;
    rope_set(new_nodes[i], rope_length(left), NULL, NULL, left, right);
    left->parent = new_nodes[i];
    right->parent = new_nodes[i];
  }

  // handle if length is odd by adding last node as parent node
  if (length % 2 != 0) {
    RopeNode* left = nodes[length - 1];
    new_nodes[new_length - 1] = malloc(sizeof(RopeNode));
    if (new_nodes[new_length - 1] == NULL) goto cleanup;
    rope_set(new_nodes[new_length - 1], rope_length(left), NULL, NULL, left, NULL);
    left->parent = new_nodes[new_length - 1];
  }

  free(nodes);
  return rope_merge(new_nodes, new_length);

  // cleanup new nodes if memory allocation fails
 cleanup:
  SDL_SetError("Failed to allocate memory in rope_merge");
  for (int i = 0; i < new_length; i++) {
    free(new_nodes[i]);
  }
  free(new_nodes);
  free(nodes);
  return NULL;
}

RopeNode *rope_build(uint32_t *text, int length)
{
  // handle empty case
  if (length == 0) {
    RopeNode *empty_node = malloc(sizeof(RopeNode));
    if (empty_node == NULL) {
      SDL_SetError("Failed to allocate memory for empty node");
      return NULL;
    }
    rope_set(empty_node, 0, NULL, NULL, NULL, NULL);
    return empty_node;
  }
  
  // guard against negative length
  if (length < 0) {
    SDL_SetError("Cannot build a rope with negative length");
    return NULL;
  }

  // find amount of leaves and allocate memory for them
  int new_length = (length + LEAF_WEIGHT - 1) / LEAF_WEIGHT;
  RopeNode **new_nodes = malloc(new_length * sizeof(RopeNode*));
  if (new_nodes == NULL) {
    SDL_SetError("Failed to allocate memory in rope_build");
    return NULL;
  }

  // null initialize array of new nodes
  for (int i = 0; i < new_length; i++) {
    new_nodes[i] = NULL;
  }

  // memory pointer for each leaf's text
  uint32_t *split_text = NULL;

  // iterate through text and create new leaves
  for (int i = 0; i < new_length; i++) {
    // handle weight if there is a remainder
    int weight = LEAF_WEIGHT;
    if (i == new_length - 1 && length % LEAF_WEIGHT != 0) {
      weight = length % LEAF_WEIGHT;
    }

    // allocate new rope node
    new_nodes[i] = malloc(sizeof(RopeNode));
    if (new_nodes[i] == NULL) goto cleanup;

    // allocate memory for the leaf's text
    split_text = malloc(weight * sizeof(uint32_t));
    if (split_text == NULL) goto cleanup;

    // copy text into node and set properties
    memcpy(split_text, &text[LEAF_WEIGHT * i], weight * sizeof(uint32_t));
    rope_set(new_nodes[i], weight, split_text, NULL, NULL, NULL);
  }

  // merge all of the leaves into a rope tree recursively
  return rope_merge(new_nodes, new_length);

  // handle cleanup if memory allocation fails
 cleanup:
  SDL_SetError("Failed to allocate memory in rope_build");
  if (new_nodes != NULL) {
    for (int i = 0; i < new_length; i++) {
      rope_free(new_nodes[i]);
    }
    free(new_nodes);
  }
  free(split_text);
  return NULL;
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
    } else if (curr->right != NULL) {
      arrput(nodes, curr);
      curr = curr->right;
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
    SDL_SetError("Failed to allocate memory for node");
    return NULL;
  }

  // set two nodes to be children of new root node
  rope_set(root, rope_length(first), NULL, NULL, first, second);
  if (first != NULL) first->parent = root;
  if (second != NULL) second->parent = root;
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

  // all heap-allocated pointers here
  RopeNode *left = NULL;              // new root for the left subtree
  RopeNode *right = NULL;             // new root for the right subtree
  RopeNode *new_node = NULL;          // new copy of the node
  uint32_t *new_text = NULL;          // text for new leaves

  // recursively build the left and right subtrees
  left = rope_rebuild(root->left);
  if (root->left != NULL && left == NULL) goto cleanup;
  right = rope_rebuild(root->right);
  if (root->right != NULL && right == NULL) goto cleanup;

  // allocate memory for new root node
  new_node = malloc(sizeof(RopeNode));
  if (new_node == NULL) goto cleanup;

  // allocate memory for new text value if it is a leaf
  if (root->value != NULL) {
    new_text = malloc(root->weight * sizeof(uint32_t));
    if (new_text == NULL) goto cleanup;
    memcpy(new_text, root->value, root->weight * sizeof(uint32_t));
  }

  // update properties on new nodes
  rope_set(new_node, root->weight, new_text, root->parent, left, right);
  if (left != NULL) left->parent = new_node;
  if (right != NULL) right->parent = new_node;

  return new_node;

  // handle cleanup if memory allocation fails
 cleanup:
  SDL_SetError("Failed to allocate memory in rope_rebuild");
  rope_free(left);
  rope_free(right);
  free(new_node);
  free(new_text);
  return NULL;
}

RopeNode **rope_split(RopeNode *root, int index)
{
  // guard to check if there is a rope
  if (root == NULL) {
    SDL_SetError("No rope to split");
    return NULL;
  }

  // all heap-allocated pointers here
  RopeNode *left = NULL;                 // new left leaf for mid-node splits
  RopeNode *right = NULL;                // new right leaf for mid-node splits
  uint32_t *left_text = NULL;            // text for new left leaf
  uint32_t *right_text = NULL;           // text for new right leaf
  RopeNode *prev_new = NULL;             // state for new node created in last iteration
  uint32_t *new_text = NULL;             // text for prev_new
  RopeNode **split_nodes = NULL;         // dynamic array of new split nodes
  RopeNode *new_node = NULL;             // new node created in each iteration
  RopeNode *new_left = NULL;             // new root node for left subtree
  RopeNode *new_right = NULL;            // new root node for right subtree
  RopeNode **new_roots = NULL;           // array of the roots of the two ropes after split
  RopeNode *split_right = NULL;          // the root of the right rope after split

  // if index is -1, return the full rope
  if (index == -1) {
    new_roots = malloc(2 * sizeof(RopeNode*));
    if (new_roots == NULL) goto cleanup;
    new_roots[0] = NULL;
    new_roots[1] = root;
    return new_roots;
  }

  // obtain information about where the split point is located
  RopeIndex idx = rope_index(root, index);
  RopeNode *prev = idx.node;

  // if split point is in the middle of a leaf, create two new leaves
  if (idx.n_idx < idx.node->weight - 1) {
    // calculate weights for the leaves
    int l_weight = idx.n_idx + 1;
    int r_weight = idx.node->weight - idx.n_idx - 1;
    
    // allocate memory for all of the nodes and text values
    left = malloc(sizeof(RopeNode));
    right = malloc(sizeof(RopeNode));
    left_text = malloc(l_weight * sizeof(uint32_t));
    right_text = malloc(r_weight * sizeof(uint32_t));

    // handle memory allocation errors
    if (left == NULL || right == NULL || left_text == NULL || right_text == NULL) {
      goto cleanup;
    }
    
    // copy the split text values from the parent node to the leaves
    memcpy(left_text, idx.node->value, l_weight * sizeof(uint32_t));
    memcpy(right_text, idx.node->value+idx.n_idx+1, r_weight * sizeof(uint32_t));

    // set leaf properties
    rope_set(left, l_weight, left_text, idx.node, NULL, NULL);
    rope_set(right, r_weight, right_text, idx.node, NULL, NULL);
    left_text = NULL;
    right_text = NULL;

    // update parent node to point to the two leaves
    free(idx.node->value);
    idx.node->value = NULL;
    idx.node->left = left;
    idx.node->right = right;
    prev = idx.node->left;
  }
  
  // allocate memory for state variable of previously created node
  prev_new = malloc(sizeof(RopeNode));
  new_text = malloc(prev->weight * sizeof(uint32_t));
  if (prev_new == NULL || new_text == NULL) {
    goto cleanup;
  }

  // copy text into prev_new and set initial properties
  memcpy(new_text, prev->value, prev->weight * sizeof(uint32_t));
  rope_set(prev_new, prev->weight, new_text, NULL, NULL, NULL);
  new_text = NULL;

  // set pointer that points to the current node
  RopeNode *ptr = prev->parent;

  // split nodes while the pointer is not at the head of the tree
  while (ptr != NULL) {
    // allocate memory for new node
    new_node = malloc(sizeof(RopeNode));
    if (new_node == NULL) {
      goto cleanup;
    }

    // if came from the right, create a new node with same info
    if (ptr->right == prev) {
      // make new nodes for left subtree
      new_left = rope_rebuild(ptr->left);
      if (new_left == NULL) goto cleanup;
      rope_set(new_node, rope_length(new_left), NULL, NULL, new_left, prev_new);
      new_left->parent = new_node;
      new_left = NULL;
    }

    // otherwise if there is another tree to the right, split it off
    else if (ptr->right != NULL) {
      rope_set(new_node, rope_length(prev_new), NULL, NULL, prev_new, NULL);
      new_right = rope_rebuild(ptr->right);
      if (new_right == NULL) goto cleanup;
      new_right->parent = NULL;
      arrput(split_nodes, new_right);
      new_right = NULL;
    }

    // otherwise, just create a new node with same info
    else {
      rope_set(new_node, rope_length(prev_new), NULL, NULL, prev_new, NULL);
    }

    // update state
    prev_new->parent = new_node;
    prev_new = new_node;
    prev = ptr;
    ptr = ptr->parent;
    new_node = NULL;
  }

  // allocate memory for the two new root nodes
  new_roots = malloc(2 * sizeof(RopeNode*));
  if (new_roots == NULL) {
    goto cleanup;
  }

  // if no split nodes, exit early
  if (arrlen(split_nodes) == 0) {
    new_roots[0] = prev_new;
    new_roots[1] = NULL;
    return new_roots;
  }

  // if only one split node, set second root node to that and return
  if (arrlen(split_nodes) == 1) {
    new_roots[0] = prev_new;
    new_roots[1] = split_nodes[0];
    return new_roots;
  }

  // concat all of the split nodes into another tree
  split_right = split_nodes[0];
  split_nodes[0] = NULL;
  for (int i = 1; i < arrlen(split_nodes); i++) {
    split_right = rope_concat(split_right, split_nodes[i]);
    if (split_right == NULL) goto cleanup;
    split_nodes[i] = NULL;
  }

  // set roots and return
  new_roots[0] = prev_new;
  new_roots[1] = split_right;
  arrfree(split_nodes);
  return new_roots;

  // cleanup section if memory allocation error occurs
 cleanup:
  SDL_SetError("Failed to allocate memory during rope_split");

  // free any simple allocations
  free(left);
  free(right);
  free(left_text);
  free(right_text);
  free(new_text);
  free(new_node);
  free(new_roots);

  // free any new ropes
  rope_free(prev_new);
  rope_free(new_left);
  rope_free(new_right);
  rope_free(split_right);

  // free the array of newly created split nodes if necessary
  if (split_nodes != NULL) {
    for (int i = 0; i < arrlen(split_nodes); i++) {
      rope_free(split_nodes[i]);
    }
  }
  arrfree(split_nodes);
  return NULL;
}

RopeNode *rope_insert(RopeNode *root, uint32_t c, int idx)
{
  // allocate memory for new node
  RopeNode *insert_node = malloc(sizeof(RopeNode));
  if (insert_node == NULL) {
    SDL_SetError("Failed to allocate memory for node");
    return NULL;
  }

  // allocate memory for text value
  uint32_t *text = malloc(sizeof(uint32_t));
  if (text == NULL) {
    SDL_SetError("Failed to allocate memory for text");
    free(insert_node);
    return NULL;
  }
  *text = c;

  // set the new node's properties
  rope_set(insert_node, 1, text, NULL, NULL, NULL);

  // split the rope at the index
  RopeNode **roots = rope_split(root, idx);

  // concatenate (left + insert node + right)
  RopeNode *new_root = rope_concat(roots[0], insert_node);
  new_root = rope_concat(new_root, roots[1]);
  free(roots);
  return new_root;
}

