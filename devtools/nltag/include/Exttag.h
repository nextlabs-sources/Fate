#pragma  once


/* Function Pointers to Resource Attribute Functions */
typedef int          (*create_mgr)  (ResourceAttributeManager **);
typedef int          (*read_attrs)  (ResourceAttributeManager *, WCHAR *fname, ResourceAttributes *attrs);
typedef int          (*write_attrs) (ResourceAttributeManager *, WCHAR *fname, ResourceAttributes *attrs);
typedef int          (*remove_attrs)(ResourceAttributeManager *, WCHAR *fname, ResourceAttributes *attrs);
typedef int          (*alloc_attrs) (ResourceAttributes **);
typedef void         (*free_attrs)  (ResourceAttributes *);
typedef int          (*get_count)   (const ResourceAttributes *);
typedef const WCHAR* (*get_name)    (const ResourceAttributes *, int index);
typedef const WCHAR* (*get_value)   (const ResourceAttributes *, int index);
typedef const WCHAR* (*get_value)   (const ResourceAttributes *, int index);
typedef void         (*add_attrs)   (ResourceAttributes *, const WCHAR *name, const WCHAR *value);

/* Structure to store pointers to functions */
typedef struct
{
	/* Functions from ResAttrMgr */
	create_mgr create_mgr_fn;
	read_attrs read_attrs_fn;
	write_attrs write_attrs_fn;
	remove_attrs remove_attrs_fn;

	/* Functions from ResAttrLib */
	alloc_attrs alloc_attrs_fn;
	free_attrs free_attrs_fn;
	get_count get_count_fn;
	get_name get_name_fn;
	get_value get_value_fn;
	add_attrs add_attrs_fn;
} resAttrPtrs;