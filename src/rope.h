#ifndef ROPE_H
#define ROPE_H

#include <stdint.h>

#define LEAF_WEIGHT 4

typedef struct RopeNode {
  int weight;
  uint32_t *value;
  struct RopeNode *parent;
  struct RopeNode *left;
  struct RopeNode *right;
} RopeNode;

typedef struct RopeIndex {
  struct RopeNode *node;
  uint32_t c;
  int n_idx;
} RopeIndex;

void rope_set(RopeNode *node, int w, uint32_t *val, RopeNode *par, RopeNode *l,
              RopeNode *r);

RopeNode *rope_merge(RopeNode **nodes, int length);

RopeNode *rope_build(uint32_t *text, int length);

RopeNode **rope_collect(RopeNode *root);

uint32_t *rope_text(RopeNode *root);

void rope_free(RopeNode *root);

int rope_length(RopeNode *root);

int rope_height(RopeNode *root, int curr_height);

RopeNode *rope_concat(RopeNode *first, RopeNode *second);

RopeIndex rope_index(RopeNode *root, int index);

RopeNode *rope_rebuild(RopeNode *root);

RopeNode **rope_split(RopeNode *root, int index);

RopeNode *rope_insert(RopeNode *root, uint32_t c, int idx);

#endif // ROPE_H
