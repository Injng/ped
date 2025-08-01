@register
class RopeTree(GenericCommand):
    """Visualize a rope binary tree."""
    _cmdline_ = "ropetree"
    _syntax_ = f"{_cmdline_} [root]"

    def do_invoke(self, argv):
        root_ptr_str = argv[0]
        root_ptr = gdb.parse_and_eval(root_ptr_str)
        self.print_tree_recursive(root_ptr, "", True)

    def print_tree_recursive(self, node_ptr, prefix, is_left):
        # base case
        if node_ptr == 0:
            return

        # deference the node and get relevant properties
        node = node_ptr.dereference()
        ref_count = node['ref_count']
        left_ptr = node['left']
        right_ptr = node['right']

        # print the right child first to make the tree view more intuitive
        self.print_tree_recursive(right_ptr, prefix + ("|   " if is_left else "    "), False)

        # print the current node
        print(f"{prefix}{'└── ' if is_left else '┌── '} refc: {ref_count}")

        # print the left child
        self.print_tree_recursive(left_ptr, prefix + ("    " if is_left else "|   "), True)
