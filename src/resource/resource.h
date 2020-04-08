#ifndef __RESOURCE_readaratus_H__
#define __RESOURCE_readaratus_H__

#include <gio/gio.h>

extern GResource *readaratus_get_resource (void);

extern void readaratus_register_resource (void);
extern void readaratus_unregister_resource (void);

#endif
