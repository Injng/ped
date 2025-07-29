#ifndef ROPE_H
#define ROPE_H

#include <stdint.h>

// Determines the size of each leaf upon rebuild of the rope.
#define LEAF_WEIGHT 4

/**
 * struct RopeNode - Defines a node within a rope.
 *
 * @weight: The weight of the rope node.
 * @value: An array of unicode codepoints representing text, if the node is a
 * leaf
 * @parent: The parent of the node.
 * @left: The left child of the node.
 * @right: The right child of the node.
 *
 * This struct represents a node within a rope binary tree that represents
 * a line of text. The weight is calculated as the total length of all the text
 * within the left subtree of the node. If the node is a leaf, that means it
 * contains a value, which is that leaf's segment of text represented as an
 * array of unicode codepoints.
 */
typedef struct RopeNode {
  int weight;
  uint32_t *value;
  struct RopeNode *parent;
  struct RopeNode *left;
  struct RopeNode *right;
} RopeNode;

/**
 * struct RopeIndex - Contains information about a character within a rope.
 *
 * @node: The leaf node containing the character.
 * @c: The unicode codepoint for the character.
 * @n_idx: The index of the character within the leaf text array.
 *
 * This struct is meant to be returned from indexing a rope by considering
 * all of it's text values as one contiguous sequence. The node returned is
 * the node containing the character that is represented by the index.
 */
typedef struct RopeIndex {
  struct RopeNode *node;
  uint32_t c;
  int n_idx;
} RopeIndex;

/**
 * rope_set() - Helper function to set properties of a rope node.
 *
 * @node: The node to set properties for.
 * @w: The weight of the node.
 * @val: The value of the node.
 * @par: The parent of the node.
 * @l: The left child of the node.
 * @r: The right child of the node.
 *
 * This is a helper function to batch set multiple properties of a node at once.
 */
void rope_set(RopeNode *node, int w, uint32_t *val, RopeNode *par, RopeNode *l,
              RopeNode *r);

/**
 * rope_merge() - Merges a list of nodes into a binary tree.
 *
 * @nodes: A heap-allocated array of rope nodes.
 * @length: Number of nodes in the array.
 *
 * This function takes in an array of rope nodes and recursively merges
 * them until a rope binary tree is formed. It then returns the root of the
 * rope, and will free the passed in node array itself. This function returns
 * NULL if it fails. For error information, use SDL_GetError().
 */
RopeNode *rope_merge(RopeNode **nodes, int length);

/**
 * rope_build() - Builds a rope given an array of text.
 *
 * @text: The array of unicode codepoints representing text.
 * @length: The length of the array.
 *
 * This function takes in an array of unicode codepoints and creates a
 * rope, returning a pointer to the root node of that rope. It first creates
 * an array of leaves representing the text, and then recursively calls
 * rope_merge() to build the rope. The rope must be freed using rope_free()
 * when it is no longer used. This function returns NULL if it fails.
 * For error information, use SDL_GetError().
 */
RopeNode *rope_build(uint32_t *text, int length);

/**
 * rope_collect() - Collects all of the leaves of a rope.
 *
 * @root: The root node of the rope.
 *
 * This function takes in a pointer to the root of a rope and traverses
 * the rope to collect all of the leaves of the rope. It returns a
 * dynamic array of the leaves.
 */
RopeNode **rope_collect(RopeNode *root);

/**
 * rope_text() - Collects all of the text of a rope.
 *
 * @root: The root node of the rope.
 *
 * This function takes in a pointer to the root of a rope and returns
 * a dynamic array of unicode codepoints representing all of the text stored
 * in the leaves of the rope in order.
 */
uint32_t *rope_text(RopeNode *root);

/**
 * rope_free() - Frees the rope from memory.
 *
 * @root: The root node of the rope.
 *
 * This function frees the rope that is pointed to by the root node using
 * a depth-first traversal of the rope, freeing each node recursively.
 */
void rope_free(RopeNode *root);

/**
 * rope_length() - Returns the length of the text in the rope.
 *
 * @root: The root node of the rope.
 *
 * This function returns the total length of all the text in the rope
 * by recursively adding the weights of the nodes down the right spine
 * of the rope.
 */
int rope_length(RopeNode *root);

/**
 * rope_height() - Returns the height of the rope.
 *
 * @root: The root node of the rope.
 * @curr_height: The starting height of the rope.
 *
 * This function returns the total height of the rope by recursively
 * calling itself on subtrees. To find the total height without any
 * previous offset, pass 0 to curr_height.
 */
int rope_height(RopeNode *root, int curr_height);

/**
 * rope_concat() - Concatenates two ropes together.
 *
 * @first: The root node of the first rope.
 * @second: The root node of the second rope.
 *
 * This function concatenates two ropes together by assigning them as
 * the left and right subtrees of a new root node. This function returns
 * NULl if it fails. For error information, use SDL_GetError().
 */
RopeNode *rope_concat(RopeNode *first, RopeNode *second);

/**
 * rope_index() - Indexes the rope and returns relevant information.
 *
 * @root: The root node of the rope.
 * @index: The index of the character in the text of the rope.
 *
 * This function returns relevant information about the leaf node containing
 * the character the index points to. The index is a zero-indexed value
 * pointing to the character if all the text within the rope is represented
 * as a contiguous array. The return value is a RopeIndex struct containing
 * the node with the text, and the index of the character within the node's
 * segment of the text.
 */
RopeIndex rope_index(RopeNode *root, int index);

/**
 * rope_rebuild() - Rebuilds a rope by replacing it with new nodes.
 *
 * @root: The root node of the rope.
 *
 * This function takes the root node of an existing rope, and makes a
 * new version of it by copying all of the values into a new rope with
 * separate nodes. A pointer to the root node of the new rope is returned.
 * This function returns NULL if it fails. For error information, use
 * SDL_GetError().
 */
RopeNode *rope_rebuild(RopeNode *root);

/**
 * rope_split() - Splits a rope into two ropes at the given index.
 *
 * @root: The root node of the rope.
 * @index: The index to split after.
 *
 * This function splits an existing rope at the point after the character
 * pointed to by the index. If the index points to a character in the
 * middle of a leaf node, two new leaf nodes will be created. This function
 * returns an array consisting of two elements, pointers to the two new
 * split ropes. This function returns NULL if it fails. For error information,
 * use SDL_GetError().
 */
RopeNode **rope_split(RopeNode *root, int index);

/**
 * rope_insert() - Inserts a character into a rope at the given index.
 *
 * @root: The root node of the rope.
 * @c: The unicode codepoint representing the character.
 * @idx: The index after which to insert the character.
 *
 * This function inserts a character represented by a unicode codepoint
 * into a rope after the point denoted by the given index. It splits the
 * rope into two new ropes at the index, and then does two concatenation
 * operates to insert the new character. The function then returns a
 * pointer to the root node. This function returns NULL if it fails. For
 * error information, use SDL_GetError().
 */
RopeNode *rope_insert(RopeNode *root, uint32_t c, int idx);

#endif // ROPE_H
