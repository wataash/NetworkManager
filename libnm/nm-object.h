// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright (C) 2007 - 2008 Novell, Inc.
 * Copyright (C) 2007 - 2012 Red Hat, Inc.
 */

#ifndef __NM_OBJECT_H__
#define __NM_OBJECT_H__

#if !defined (__NETWORKMANAGER_H_INSIDE__) && !defined (NETWORKMANAGER_COMPILATION)
#error "Only <NetworkManager.h> can be included directly."
#endif

#include "nm-types.h"

G_BEGIN_DECLS

#define NM_TYPE_OBJECT            (nm_object_get_type ())
#define NM_OBJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NM_TYPE_OBJECT, NMObject))
#define NM_OBJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NM_TYPE_OBJECT, NMObjectClass))
#define NM_IS_OBJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NM_TYPE_OBJECT))
#define NM_IS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NM_TYPE_OBJECT))
#define NM_OBJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NM_TYPE_OBJECT, NMObjectClass))

#define NM_OBJECT_PATH "path"
#define NM_OBJECT_DBUS_CONNECTION "dbus-connection"
#define NM_OBJECT_DBUS_OBJECT "dbus-object"
#define NM_OBJECT_DBUS_OBJECT_MANAGER "dbus-object-manager"

/**
 * NMObject:
 */
struct _NMObject {
	GObject parent;
};

typedef struct {
	GObjectClass parent;

	/* Methods */
	void (*init_dbus) (NMObject *object);

	/* The "object-creation-failed" method is PRIVATE for libnm and
	 * is not meant for any external usage.  It indicates that an error
	 * occurred during creation of an object.
	 */
	void (*object_creation_failed) (NMObject *master_object,
	                                const char *failed_path);

	/*< private >*/
	gpointer padding[8];
} NMObjectClass;

GType nm_object_get_type (void);

const char      *nm_object_get_path            (NMObject *object);

G_END_DECLS

#endif /* __NM_OBJECT_H__ */
