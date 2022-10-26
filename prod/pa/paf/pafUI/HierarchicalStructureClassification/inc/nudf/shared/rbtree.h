#pragma once

#ifdef __cplusplus
extern "C"{
#endif

#pragma pack(push, 8)

typedef struct _rb_node
{
	ULONG_PTR		rb_parent_color;
#define	RB_RED		0
#define	RB_BLACK	1
	struct _rb_node	*rb_right;
	struct _rb_node	*rb_left;
}rb_node;


typedef struct _rb_root
{
	rb_node *rb_node;

}rb_root;


#define rb_parent(r)		((rb_node *)((r)->rb_parent_color & ~3))
#define rb_color(r)			((r)->rb_parent_color & 1)
#define rb_is_red(r)		(!rb_color(r))
#define rb_is_black(r)		rb_color(r)
#define rb_set_red(r)		do { (r)->rb_parent_color &= ~1; } while (0)
#define rb_set_black(r)		do { (r)->rb_parent_color |= 1; } while (0)

static __inline void rb_set_parent(rb_node *rb, rb_node *p)
{
	rb->rb_parent_color = (rb->rb_parent_color & 3) | (ULONG_PTR)p;
}
static __inline void rb_set_color(rb_node *rb, ULONG_PTR color)
{
	rb->rb_parent_color = (rb->rb_parent_color & ~1) | color;
}

#define	rb_entry(ptr, type, member)		((type*)((DWORD_PTR)ptr - (DWORD_PTR)&(((type*)0)->member)))

#define RB_EMPTY_ROOT(root)					((root)->rb_node == NULL)
#define RB_EMPTY_NODE(node)					(rb_parent(node) == node)
#define RB_CLEAR_NODE(node)					(rb_set_parent(node, node))
#define RB_EACH_NODE(node, root)			for (node=rb_first(root); node; node=rb_next(node))
#define RB_EACH_NODE_SAFE(node, temp, root)	for (node=rb_first(root), temp=node?rb_next(node):NULL;  node;  node=temp, temp=node?rb_next(node):NULL)

extern void rb_insert_color(rb_node *, rb_root *);
extern void rb_erase(rb_node *, rb_root *);

extern rb_node *rb_next(rb_node *);
extern rb_node *rb_prev(rb_node *);
extern rb_node *rb_first(rb_root *);
extern rb_node *rb_last(rb_root *);

extern void rb_replace_node(rb_node *victim, rb_node *newnode,rb_root *root);

static __inline void rb_link_node(rb_node * node, rb_node * parent, rb_node ** rb_link)
{
	node->rb_parent_color = (ULONG_PTR)parent;
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}

#pragma pack(pop)

#ifdef __cplusplus
}
#endif