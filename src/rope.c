#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL_error.h>

#include "rope.h"
#include "stb_ds.h"

void rope_set(RopeNode *node, int w, int refc, uint32_t *val, RopeNode *l, RopeNode *r)
{
  node->weight = w;
  node->ref_count = refc;
  node->value = val;
  node->left = l;
  node->right = r;
  if (node->left != NULL) node->left->ref_count++;
  if (node->right != NULL) node->right->ref_count++;
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
    if (new_nodes[i] == NULL) goto cleanup;
    rope_set(new_nodes[i], rope_length(left), 1, NULL, left, right);
  }

  // handle if length is odd by adding last node as parent node
  if (length % 2 != 0) {
    RopeNode* left = nodes[length - 1];
    new_nodes[new_length - 1] = malloc(sizeof(RopeNode));
    if (new_nodes[new_length - 1] == NULL) goto cleanup;
    rope_set(new_nodes[new_length - 1], rope_length(left), 1, NULL, left, NULL);
  }

  rope_arr_free(nodes, length);
  return rope_merge(new_nodes, new_length);

  // cleanup new nodes if memory allocation fails
 cleanup:
  SDL_SetError("Failed to allocate memory in rope_merge");
  rope_arr_free(new_nodes, new_length);
  rope_arr_free(nodes, length);
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
    rope_set(empty_node, 0, 1, NULL, NULL, NULL);
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
    rope_set(new_nodes[i], weight, 1, split_text, NULL, NULL);
  }

  // merge all of the leaves into a rope tree recursively
  return rope_merge(new_nodes, new_length);

  // handle cleanup if memory allocation fails
 cleanup:
  SDL_SetError("Failed to allocate memory in rope_build");
  rope_arr_free(new_nodes, new_length);
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
      curr->ref_count++;
      curr = NULL;
      while (curr == NULL && arrlen(nodes) > 0) {
        curr = arrlast(nodes)->right;
        arrdel(nodes, arrlen(nodes) - 1);
      }
    }
  }
  
  // free the dynamic arrays and return the leaves
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
    leaf->ref_count--;
    uint32_t *value = leaf->value;
    for (int j = 0; j < leaf->weight; j++) {
      arrput(text, value[j]);
    }
  }

  arrfree(leaves);
  return text;
}

void rope_deref(RopeNode *node)
{
  // guard for null node and decrement ref count
  if (node == NULL) return;
  node->ref_count--;

  // if ref count is zero, it is safe to free this node
  if (node->ref_count == 0) {
    // unreference the children
    rope_deref(node->left);
    rope_deref(node->right);

    // free the node
    free(node->value);
    free(node);
  }
}

void rope_arr_free(RopeNode **arr, int length)
{
  // exit if array is NULL
  if (arr == NULL) return;

  // dereference each node inside of the array and free
  for (int i = 0; i < length; i++) {
    rope_deref(arr[i]);
  }
  free(arr);
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
  rope_set(root, rope_length(first), 1, NULL, first, second);
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

RopeNode **rope_split(RopeNode *root, int index)
{
  // all heap-allocated pointers here
  RopeNode **new_roots = NULL;          // array of the roots of the two ropes after split
  uint32_t *text_left = NULL;           // text to the left of the split in the leaf
  uint32_t *text_right = NULL;          // text to the right of the split in the leaf

  // if index is -1, return whole rope as right tree
  if (index == -1) {
    // allocate memory for new root pointers
    new_roots = malloc(2 * sizeof(RopeNode*));
    if (new_roots == NULL) goto cleanup;
    
    new_roots[0] = rope_build(NULL, 0);
    if (new_roots[0] == NULL) goto cleanup;
    new_roots[1] = root;
    root->ref_count++;
    return new_roots;
  }
  
  // base case: node is leaf
  if (root->left == NULL && root->right == NULL) {
    // allocate memory for new root pointers
    new_roots = malloc(2 * sizeof(RopeNode*));
    if (new_roots == NULL) goto cleanup;
    
    // if split point is at end of leaf, then return empty right rope
    if (root->weight - 1 == index) {
      new_roots[0] = root;
      root->ref_count++;
      new_roots[1] = rope_build(NULL, 0);
      if (new_roots[1] == NULL) goto cleanup;
      return new_roots;
    }

    // otherwise calculate weights of new split left and right nodes
    int left_weight = index + 1;
    int right_weight =  root->weight - index - 1;

    // allocate memory for new text and copy slices from old value
    text_left = malloc(left_weight * sizeof(uint32_t));
    text_right = malloc(right_weight * sizeof(uint32_t));
    if (text_left == NULL || text_right == NULL) goto cleanup;
    memcpy(text_left, root->value, left_weight * sizeof(uint32_t));
    memcpy(text_right, root->value+index+1, right_weight * sizeof(uint32_t));

    // create new left and right nodes from the split text
    new_roots[0] = malloc(sizeof(RopeNode));
    new_roots[1] = malloc(sizeof(RopeNode));
    if (new_roots[0] == NULL || new_roots[1] == NULL) goto cleanup;
    rope_set(new_roots[0], left_weight, 1, text_left, NULL, NULL);
    rope_set(new_roots[1], right_weight, 1, text_right, NULL, NULL);

    return new_roots;
  }

  // otherwise split down left or right subtree based on index
  if (index >= root->weight) {
    new_roots = rope_split(root->right, index - root->weight);
    if (new_roots == NULL) goto cleanup;
    new_roots[0]->ref_count--;
    new_roots[0] = rope_concat(root->left, new_roots[0]);
    if (new_roots[0] == NULL) goto cleanup;
  } else {
    new_roots = rope_split(root->left, index);
    if (new_roots == NULL) goto cleanup;
    new_roots[1]->ref_count--;
    new_roots[1] = rope_concat(new_roots[1], root->right);
    if (new_roots[1] == NULL) goto cleanup;
  }
  return new_roots;

  // memory cleanup for allocation failure
 cleanup:
  SDL_SetError("Failed to allocate memory during rope_split");
  rope_arr_free(new_roots, 2);
  free(text_left);
  free(text_right);
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
  rope_set(insert_node, 1, 1, text, NULL, NULL);

  // split the rope at the index
  RopeNode **roots = rope_split(root, idx);
  if (roots == NULL) {
    rope_deref(insert_node);
    return NULL;
  }

  // concatenate (left + insert node + right)
  RopeNode *full_concat = rope_concat(roots[0], insert_node);
  if (full_concat == NULL) {
    rope_deref(insert_node);
    rope_arr_free(roots, 2);
    return NULL;
  }

  // if roots[1] is an empty rope, skip the concat
  if (roots[1]->weight != 0) {
    RopeNode *prev_concat = full_concat;
    full_concat = rope_concat(full_concat, roots[1]);
    rope_deref(prev_concat);
    if (full_concat == NULL) {
      rope_deref(insert_node);
      rope_arr_free(roots, 2);
      return NULL;
    }
  }

  // cleanup
  rope_deref(insert_node);
  rope_arr_free(roots, 2);
  return full_concat;
}

RopeNode *rope_delete(RopeNode *root, int idx)
{
  // split at point before and at the index
  RopeNode **before_splits = rope_split(root, idx - 1);
  if (before_splits == NULL) return NULL;
  RopeNode **after_splits = rope_split(root, idx);
  if (after_splits == NULL) goto cleanup;

  // make new rope without the deleted character
  RopeNode *new_rope = rope_concat(before_splits[0], after_splits[1]);
  if (new_rope == NULL) goto cleanup;

  // free unused rope nodes
  rope_arr_free(before_splits, 2);
  rope_arr_free(after_splits, 2);
  return new_rope;

  // cleanup logic if memory allocation fails
 cleanup:
  rope_arr_free(before_splits, 2);
  rope_arr_free(after_splits, 2);
  return NULL;
}

