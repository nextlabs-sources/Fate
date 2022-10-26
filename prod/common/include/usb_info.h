
#ifndef _USB_INFO_H_
#define _USB_INFO_H_


#define DB_DIR					".udev/db"

#define PATH_TO_NAME_CHAR			'@'

struct name_entry {
	struct list_head node;
	char name[MAX_PATH_LENGTH];
};

struct sysfs_attr {
	struct list_head node;
	char path[MAX_PATH_LENGTH];
	char *value;			/* points to value_local if value is cached */
	char value_local[MAX_NAME_LENGTH];
};

struct sysfs_device {
	struct list_head node;			/* for device cache */
	struct sysfs_device *parent;		/* already cached parent*/
	char devpath[MAX_PATH_LENGTH];
	char subsystem[MAX_NAME_LENGTH];		/* $class/$bus/"drivers" */
	char kernel_name[MAX_NAME_LENGTH];		/* device instance name */
	char kernel_number[MAX_NAME_LENGTH];
	char driver[MAX_NAME_LENGTH];			/* device driver name */
};

struct udevice {
	/* device event */
	struct sysfs_device *dev;		/* points to dev_local by default */
	struct sysfs_device dev_local;
	struct sysfs_device *dev_parent;	/* current parent device used for matching */
	char action[MAX_NAME_LENGTH];

	/* node */
	char name[MAX_PATH_LENGTH];
	struct list_head symlink_list;
	int symlink_final;
	char owner[MAX_USER_LENGTH];
	int owner_final;
	char group[MAX_USER_LENGTH];
	int group_final;
	mode_t mode;
	int mode_final;
	dev_t devt;

	/* event processing */
	struct list_head run_list;
	int run_final;
	struct list_head env_list;
	char tmp_node[MAX_PATH_LENGTH];
	int partitions;
	int ignore_device;
	int ignore_remove;
	char program_result[MAX_PATH_LENGTH];
	int test_run;
};


#endif


