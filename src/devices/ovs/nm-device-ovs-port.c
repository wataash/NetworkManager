// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Red Hat, Inc.
 */

#include "nm-default.h"

#include "nm-device-ovs-port.h"
#include "nm-device-ovs-interface.h"
#include "nm-ovsdb.h"

#include "devices/nm-device-private.h"
#include "nm-active-connection.h"
#include "nm-setting-connection.h"
#include "nm-setting-ovs-port.h"
#include "nm-setting-ovs-port.h"

#include "devices/nm-device-logging.h"
_LOG_DECLARE_SELF (NMDeviceOvsPort);

/*****************************************************************************/

struct _NMDeviceOvsPort {
	NMDevice parent;
};

struct _NMDeviceOvsPortClass {
	NMDeviceClass parent;
};

G_DEFINE_TYPE (NMDeviceOvsPort, nm_device_ovs_port, NM_TYPE_DEVICE)

/*****************************************************************************/

static const char *
get_type_description (NMDevice *device)
{
	return "ovs-port";
}

static gboolean
create_and_realize (NMDevice *device,
                    NMConnection *connection,
                    NMDevice *parent,
                    const NMPlatformLink **out_plink,
                    GError **error)
{
	/* The port will be added to ovsdb when an interface is enslaved,
	 * because there's no such thing like an empty port. */

	return TRUE;
}

static NMDeviceCapabilities
get_generic_capabilities (NMDevice *device)
{
	return NM_DEVICE_CAP_IS_SOFTWARE;
}

static NMActStageReturn
act_stage3_ip_config_start (NMDevice *device,
                            int addr_family,
                            gpointer *out_config,
                            NMDeviceStateReason *out_failure_reason)
{
	return NM_ACT_STAGE_RETURN_IP_FAIL;
}

static void
add_iface_cb (GError *error, gpointer user_data)
{
	NMDevice *slave = user_data;

	if (   error
	    && !g_error_matches (error, NM_UTILS_ERROR, NM_UTILS_ERROR_CANCELLED_DISPOSING)) {
		nm_log_warn (LOGD_DEVICE, "device %s could not be added to a ovs port: %s",
		             nm_device_get_iface (slave), error->message);
		nm_device_state_changed (slave,
		                         NM_DEVICE_STATE_FAILED,
		                         NM_DEVICE_STATE_REASON_OVSDB_FAILED);
	}

	g_object_unref (slave);
}

static gboolean
enslave_slave (NMDevice *device, NMDevice *slave, NMConnection *connection, gboolean configure)
{
	NMActiveConnection *ac_port = NULL;
	NMActiveConnection *ac_bridge = NULL;

	if (!configure)
		return TRUE;

	ac_port = NM_ACTIVE_CONNECTION (nm_device_get_act_request (device));
	ac_bridge = nm_active_connection_get_master (ac_port);
	if (!ac_bridge)
		ac_bridge = ac_port;

	nm_ovsdb_add_interface (nm_ovsdb_get (),
	                        nm_active_connection_get_applied_connection (ac_bridge),
	                        nm_device_get_applied_connection (device),
	                        nm_device_get_applied_connection (slave),
	                        add_iface_cb, g_object_ref (slave));

	return TRUE;
}

static void
del_iface_cb (GError *error, gpointer user_data)
{
	NMDevice *slave = user_data;

	if (   error
	    && !g_error_matches (error, NM_UTILS_ERROR, NM_UTILS_ERROR_CANCELLED_DISPOSING)) {
		nm_log_warn (LOGD_DEVICE, "device %s could not be removed from a ovs port: %s",
		             nm_device_get_iface (slave), error->message);
		nm_device_state_changed (slave,
		                         NM_DEVICE_STATE_FAILED,
		                         NM_DEVICE_STATE_REASON_OVSDB_FAILED);
	}

	g_object_unref (slave);
}

static void
release_slave (NMDevice *device, NMDevice *slave, gboolean configure)
{
	NMDeviceOvsPort *self = NM_DEVICE_OVS_PORT (device);

	if (configure) {
		_LOGI (LOGD_DEVICE, "releasing ovs interface %s", nm_device_get_ip_iface (slave));
		nm_ovsdb_del_interface (nm_ovsdb_get (), nm_device_get_iface (slave),
		                        del_iface_cb, g_object_ref (slave));
		/* Open VSwitch is going to delete this one. We must ignore what happens
		 * next with the interface. */
		if (NM_IS_DEVICE_OVS_INTERFACE (slave))
			nm_device_update_from_platform_link (slave, NULL);
	} else
		_LOGI (LOGD_DEVICE, "ovs interface %s was released", nm_device_get_ip_iface (slave));
}

/*****************************************************************************/

static void
nm_device_ovs_port_init (NMDeviceOvsPort *self)
{
}

static const NMDBusInterfaceInfoExtended interface_info_device_ovs_port = {
	.parent = NM_DEFINE_GDBUS_INTERFACE_INFO_INIT (
		NM_DBUS_INTERFACE_DEVICE_OVS_PORT,
		.properties = NM_DEFINE_GDBUS_PROPERTY_INFOS (
			NM_DEFINE_DBUS_PROPERTY_INFO_EXTENDED_READABLE ("Slaves", "ao", NM_DEVICE_SLAVES),
		),
		.signals = NM_DEFINE_GDBUS_SIGNAL_INFOS (
			&nm_signal_info_property_changed_legacy,
		),
	),
	.legacy_property_changed = TRUE,
};

static void
nm_device_ovs_port_class_init (NMDeviceOvsPortClass *klass)
{
	NMDBusObjectClass *dbus_object_class = NM_DBUS_OBJECT_CLASS (klass);
	NMDeviceClass *device_class = NM_DEVICE_CLASS (klass);

	dbus_object_class->interface_infos = NM_DBUS_INTERFACE_INFOS (&interface_info_device_ovs_port);

	device_class->connection_type_supported = NM_SETTING_OVS_PORT_SETTING_NAME;
	device_class->connection_type_check_compatible = NM_SETTING_OVS_PORT_SETTING_NAME;
	device_class->link_types = NM_DEVICE_DEFINE_LINK_TYPES ();

	device_class->is_master = TRUE;
	device_class->get_type_description = get_type_description;
	device_class->create_and_realize = create_and_realize;
	device_class->get_generic_capabilities = get_generic_capabilities;
	device_class->act_stage3_ip_config_start = act_stage3_ip_config_start;
	device_class->enslave_slave = enslave_slave;
	device_class->release_slave = release_slave;
}
