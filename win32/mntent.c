/*
 * A simple WIN32 implementation of mntent routines.  It only handles
 * fixed logical drives.
 */
#include "libbb.h"

struct mntdata {
	DWORD flags;
	int index;
};

FILE *setmntent(const char *file UNUSED_PARAM, const char *mode UNUSED_PARAM)
{
	struct mntdata *data;

	if ( (data=malloc(sizeof(struct mntdata))) == NULL ) {
		return NULL;
	}

	data->flags = GetLogicalDrives();
	data->index = -1;

	return (FILE *)data;
}

struct mntent *getmntent(FILE *stream)
{
	struct mntdata *data = (struct mntdata *)stream;
	static char mnt_fsname[4];
	static char mnt_dir[4];
	static struct mntent my_mount_entry =
					{ mnt_fsname, mnt_dir, (char *)"", (char *)"", 0, 0 };
	struct mntent *entry;
	static char fsname[100];

	entry = NULL;
	while ( ++data->index < 26 ) {
		if ( (data->flags & 1<<data->index) != 0 ) {
			mnt_fsname[0] = 'A' + data->index;
			mnt_fsname[1] = ':';
			mnt_fsname[2] = '\0';
			mnt_dir[0] = 'A' + data->index;
			mnt_dir[1] = ':';
			mnt_dir[2] = '\\';
			mnt_dir[3] = '\0';

			if ( GetDriveType(mnt_dir) == DRIVE_FIXED ) {
				my_mount_entry.mnt_type = "";
				if ( GetVolumeInformation(mnt_dir, NULL, 0, NULL, NULL,
								NULL, fsname, 100) ) {
					my_mount_entry.mnt_type = fsname;
				}

				entry = &my_mount_entry;
				break;
			}
		}
	}

	return entry;
}

int endmntent(FILE *stream)
{
	free(stream);
	return 0;
}
