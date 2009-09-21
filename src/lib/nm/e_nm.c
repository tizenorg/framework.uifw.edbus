#include "E_Nm.h"
#include "e_nm_private.h"
#include "e_dbus_private.h"
#include <string.h>

static const Property properties[] = {
  { .name = "WirelessEnabled", .sig = "b", .offset = offsetof(E_NM, wireless_enabled) },
  { .name = "WirelessHardwareEnabled", .sig = "b", .offset = offsetof(E_NM, wireless_hardware_enabled) },
  { .name = "ActiveConnections", .sig = "ao", .offset = offsetof(E_NM, active_connections) },
  { .name = "State", .sig = "u", .offset = offsetof(E_NM, state) },
  { .name = NULL }
};
 
static void
cb_state_changed(void *data, DBusMessage *msg)
{
  E_NM_Internal *nmi;
  unsigned int   state;
  DBusError      err;
  if (!msg || !data) return;

  nmi = data;
  dbus_error_init(&err);
  dbus_message_get_args(msg, &err, DBUS_TYPE_UINT32, &state, DBUS_TYPE_INVALID);
  if (dbus_error_is_set(&err))
  {
    E_DBUS_LOG_ERR("%s - %s", err.name, err.message);
    return;
  }

  nmi->nm.state = state;
  if (nmi->state_changed)
    nmi->state_changed(&(nmi->nm), state);
}

static void
cb_properties_changed(void *data, DBusMessage *msg)
{
  E_NM_Internal *nmi;
  if (!msg || !data) return;

  nmi = data;
  parse_properties(nmi, properties, msg);

  if (nmi->properties_changed)
    nmi->properties_changed(&(nmi->nm));
}

static void
cb_device_added(void *data, DBusMessage *msg)
{
  E_NM_Internal *nmi;
  const char *device;
  DBusError err;
  if (!msg || !data) return;

  nmi = data;
  dbus_error_init(&err);
  dbus_message_get_args(msg, &err, DBUS_TYPE_OBJECT_PATH, &device, DBUS_TYPE_INVALID);
  if (dbus_error_is_set(&err))
  {
    E_DBUS_LOG_ERR("%s - %s", err.name, err.message);
    return;
  }

  if (nmi->device_added)
    nmi->device_added(&(nmi->nm), device);
}

static void
cb_device_removed(void *data, DBusMessage *msg)
{
  E_NM_Internal *nmi;
  const char *device;
  DBusError err;
  if (!msg || !data) return;

  nmi = data;
  dbus_error_init(&err);
  dbus_message_get_args(msg, &err, DBUS_TYPE_OBJECT_PATH, &device, DBUS_TYPE_INVALID);
  if (dbus_error_is_set(&err))
  {
    E_DBUS_LOG_ERR("%s - %s", err.name, err.message);
    return;
  }

  if (nmi->device_removed)
    nmi->device_removed(&(nmi->nm), device);
}

EAPI int
e_nm_get(int (*cb_func)(void *data, E_NM *nm), void *data)
{
  E_NM_Internal *nmi = NULL;
  Property_Data *d = NULL;

  nmi = calloc(1, sizeof(E_NM_Internal));
  if (!nmi) goto error;
  d = calloc(1, sizeof(Property_Data));
  if (!d) goto error;
  d->nmi = nmi;
  d->cb_func = OBJECT_CB(cb_func);
  d->data = data;
  d->property = properties;
  d->service = E_NM_SERVICE;
  d->object = strdup(E_NM_PATH);
  d->interface = E_NM_INTERFACE;
  d->reply = nmi;

  nmi->conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
  if (!nmi->conn) goto error;
  nmi->handlers = ecore_list_new();
  ecore_list_append(nmi->handlers, e_nm_signal_handler_add(nmi->conn, "StateChanged", cb_state_changed, nmi));
  ecore_list_append(nmi->handlers, e_nm_signal_handler_add(nmi->conn, "PropertiesChanged", cb_properties_changed, nmi));
  ecore_list_append(nmi->handlers, e_nm_signal_handler_add(nmi->conn, "DeviceAdded", cb_device_added, nmi));
  ecore_list_append(nmi->handlers, e_nm_signal_handler_add(nmi->conn, "DeviceRemoved", cb_device_removed, nmi));

  return property_get(nmi->conn, d);

error:
  if (d) free(d);
  if (nmi) free(nmi);
  return 0;
}

EAPI void
e_nm_free(E_NM *nm)
{
  E_NM_Internal *nmi;

  if (!nm) return;
  nmi = (E_NM_Internal *)nm;
  if (nmi->nm.active_connections) ecore_list_destroy(nmi->nm.active_connections);
  if (nmi->handlers)
  {
    E_DBus_Signal_Handler *sh;

    while ((sh = ecore_list_first_remove(nmi->handlers)))
      e_dbus_signal_handler_del(nmi->conn, sh);
    ecore_list_destroy(nmi->handlers);
  }
  e_dbus_connection_close(nmi->conn);
  free(nmi);
}

EAPI void
e_nm_dump(E_NM *nm)
{
  const char *conn;

  if (!nm) return;
  INFO("E_NM:");
  INFO("wireless_enabled         : %d", nm->wireless_enabled);
  INFO("wireless_hardware_enabled: %d", nm->wireless_hardware_enabled);
  INFO("active_connections       :");
  if (nm->active_connections)
  {
    ecore_list_first_goto(nm->active_connections);
    while ((conn = ecore_list_next(nm->active_connections)))
      INFO(" - %s", conn);
  }
  INFO("state                    : ");
  switch (nm->state)
  {
    case E_NM_STATE_UNKNOWN:
      INFO("E_NM_STATE_UNKNOWN");
      break;
    case E_NM_STATE_ASLEEP:
      INFO("E_NM_STATE_ASLEEP");
      break;
    case E_NM_STATE_CONNECTING:
      INFO("E_NM_STATE_CONNECTING");
      break;
    case E_NM_STATE_CONNECTED:
      INFO("E_NM_STATE_CONNECTED");
      break;
    case E_NM_STATE_DISCONNECTED:
      INFO("E_NM_STATE_DISCONNECTED");
      break;
  }
  INFO("");
}

EAPI void
e_nm_data_set(E_NM *nm, void *data)
{
  E_NM_Internal *nmi;

  nmi = (E_NM_Internal *)nm;
  nmi->data = data;
}

EAPI void *
e_nm_data_get(E_NM *nm)
{
  E_NM_Internal *nmi;

  nmi = (E_NM_Internal *)nm;
  return nmi->data;
}

EAPI void
e_nm_callback_state_changed_set(E_NM *nm, int (*cb_func)(E_NM *nm, E_NM_State state))
{
  E_NM_Internal *nmi;

  nmi = (E_NM_Internal *)nm;
  nmi->state_changed = cb_func;
}

EAPI void
e_nm_callback_properties_changed_set(E_NM *nm, int (*cb_func)(E_NM *nm))
{
  E_NM_Internal *nmi;

  nmi = (E_NM_Internal *)nm;
  nmi->properties_changed = cb_func;
}

EAPI void
e_nm_callback_device_added_set(E_NM *nm, int (*cb_func)(E_NM *nm, const char *device))
{
  E_NM_Internal *nmi;

  nmi = (E_NM_Internal *)nm;
  nmi->device_added = cb_func;
}

EAPI void
e_nm_callback_device_removed_set(E_NM *nm, int (*cb_func)(E_NM *nm, const char *device))
{
  E_NM_Internal *nmi;

  nmi = (E_NM_Internal *)nm;
  nmi->device_removed = cb_func;
}

EAPI int
e_nm_wireless_enabled_set(E_NM *nm, int enabled)
{
  E_NM_Internal *nmi;

  nmi = (E_NM_Internal *)nm;
  return e_nm_properties_set(nmi->conn, "WirelessEnabled", DBUS_TYPE_BOOLEAN, &enabled, NULL, NULL) ? 1 : 0;
}
