/*
 * This file defines functions that query each of the functions provided by
 * the org.freedesktop.NetworkManager.Device DBus interface.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include "E_Nm.h"
#include "e_nm_private.h"
#include "e_dbus_private.h"

static void property_device_type(Property_Data *data, DBusMessageIter *iter);

static const Property device_wired_properties[] = {
  { .name = "HwAddress", .sig = "s", .offset = offsetof(E_NM_Device, wired.hw_address) },
  { .name = "Speed", .sig = "u", .offset = offsetof(E_NM_Device, wired.speed) },
  { .name = "Carrier", .sig = "b", .offset = offsetof(E_NM_Device, wired.carrier) },
  { .name = NULL }
};

static const Property device_wireless_properties[] = {
  { .name = "HwAddress", .sig = "s", .offset = offsetof(E_NM_Device, wireless.hw_address) },
  { .name = "Mode", .sig = "u", .offset = offsetof(E_NM_Device, wireless.mode) },
  { .name = "Bitrate", .sig = "u", .offset = offsetof(E_NM_Device, wireless.bitrate) },
  { .name = "ActiveAccessPoint", .sig = "o", .offset = offsetof(E_NM_Device, wireless.active_access_point) },
  { .name = "WirelessCapabilities", .sig = "u", .offset = offsetof(E_NM_Device, wireless.wireless_capabilities) },
  { .name = NULL }
};

static const Property device_properties[] = {
  { .name = "Udi", .sig = "s", .offset = offsetof(E_NM_Device, udi) },
  { .name = "Interface", .sig = "s", .offset = offsetof(E_NM_Device, interface) },
  { .name = "Driver", .sig = "s", .offset = offsetof(E_NM_Device, driver) },
  { .name = "Capabilities", .sig = "u", .offset = offsetof(E_NM_Device, capabilities) },
  { .name = "Ip4Address", .sig = "u", .offset = offsetof(E_NM_Device, ip4_address) },
  { .name = "State", .sig = "u", .offset = offsetof(E_NM_Device, state) },
  { .name = "Ip4Config", .sig = "o", .offset = offsetof(E_NM_Device, ip4_config) },
  { .name = "Dhcp4Config", .sig = "o", .offset = offsetof(E_NM_Device, dhcp4_config) },
  { .name = "Managed", .sig = "b", .offset = offsetof(E_NM_Device, managed) },
  { .name = "DeviceType", .func = property_device_type, .offset = offsetof(E_NM_Device, device_type) },
  { .name = NULL }
};

static void
cb_state_changed(void *data, DBusMessage *msg)
{
  E_NM_Device_Internal *dev;
  unsigned int new_state, old_state, reason;
  DBusError err;
  if (!msg || !data) return;

  dev = data;
  dbus_error_init(&err);
  dbus_message_get_args(msg, &err,
                        DBUS_TYPE_UINT32, &new_state,
                        DBUS_TYPE_UINT32, &old_state,
                        DBUS_TYPE_UINT32, &reason,
                        DBUS_TYPE_INVALID);
  if (dbus_error_is_set(&err))
  {
    ERR("%s - %s", err.name, err.message);
    return;
  }

  dev->dev.state = new_state;
  if (dev->state_changed)
    dev->state_changed(&(dev->dev), new_state, old_state, reason);
}

static void
cb_wired_properties_changed(void *data, DBusMessage *msg)
{
  E_NM_Device_Internal *dev;
  if (!msg || !data) return;

  dev = data;
  parse_properties(dev, device_wired_properties, msg);

  if (dev->properties_changed)
    dev->properties_changed(&(dev->dev));
}

static void
cb_wireless_properties_changed(void *data, DBusMessage *msg)
{
  E_NM_Device_Internal *dev;
  if (!msg || !data) return;

  dev = data;
  parse_properties(dev, device_wireless_properties, msg);

  if (dev->properties_changed)
    dev->properties_changed(&(dev->dev));
}

static void
cb_wireless_access_point_added(void *data, DBusMessage *msg)
{
  E_NM_Device_Internal *dev;
  DBusError err;
  const char *path;
  if (!msg || !data) return;

  dev = data;
  dbus_error_init(&err);
  dbus_message_get_args(msg, &err,
                        DBUS_TYPE_OBJECT_PATH, &path,
                        DBUS_TYPE_INVALID);
  if (dbus_error_is_set(&err))
  {
    ERR("%s - %s", err.name, err.message);
    return;
  }


  if (dev->access_point_added)
    dev->access_point_added(&(dev->dev), path);
}

static void
cb_wireless_access_point_removed(void *data, DBusMessage *msg)
{
  E_NM_Device_Internal *dev;
  DBusError err;
  const char *path;
  if (!msg || !data) return;

  dev = data;
  dbus_error_init(&err);
  dbus_message_get_args(msg, &err,
                        DBUS_TYPE_OBJECT_PATH, &path,
                        DBUS_TYPE_INVALID);
  if (dbus_error_is_set(&err))
  {
    ERR("%s - %s", err.name, err.message);
    return;
  }


  if (dev->access_point_removed)
    dev->access_point_removed(&(dev->dev), path);
}

static void
cb_properties_changed(void *data, DBusMessage *msg)
{
  E_NM_Device_Internal *dev;
  if (!msg || !data) return;

  dev = data;
  parse_properties(dev, device_properties, msg);

  if (dev->properties_changed)
    dev->properties_changed(&(dev->dev));
}

static void
property_device_type(Property_Data *data, DBusMessageIter *iter)
{
  E_NM_Device_Internal *dev;

  if (!check_arg_type(iter, 'u')) goto error;

  dev = data->reply;
  dbus_message_iter_get_basic(iter, &(dev->dev.device_type));
  switch (dev->dev.device_type)
  {
    case E_NM_DEVICE_TYPE_WIRED:
      data->property = device_wired_properties;
      data->interface = E_NM_INTERFACE_DEVICE_WIRED;
      dev->handlers = eina_list_append(dev->handlers, e_nm_device_wired_signal_handler_add(data->nmi->conn, dev->dev.udi, "PropertiesChanged", cb_wired_properties_changed, dev));
      property_get(data->nmi->conn, data);
      break;
    case E_NM_DEVICE_TYPE_WIRELESS:
      data->property = device_wireless_properties;
      data->interface = E_NM_INTERFACE_DEVICE_WIRELESS;
      dev->handlers = eina_list_append(dev->handlers, e_nm_device_wireless_signal_handler_add(data->nmi->conn, dev->dev.udi, "PropertiesChanged", cb_wireless_properties_changed, dev));
      dev->handlers = eina_list_append(dev->handlers, e_nm_device_wireless_signal_handler_add(data->nmi->conn, dev->dev.udi, "AccessPointAdded", cb_wireless_access_point_added, dev));
      dev->handlers = eina_list_append(dev->handlers, e_nm_device_wireless_signal_handler_add(data->nmi->conn, dev->dev.udi, "AccessPointRemoved", cb_wireless_access_point_removed, dev));
      property_get(data->nmi->conn, data);
      break;
    default:
      if (data->cb_func) data->cb_func(data->data, dev);
      property_data_free(data);
      break;
  }
  return;
 
error:
  if (data->reply) e_nm_device_free(data->reply);
  if (data->cb_func) data->cb_func(data->data, NULL);
  property_data_free(data);
}

static void
check_done(Reply_Data *d, Eina_List *list)
{
  if (!list)
  {
    d->cb_func(d->data, NULL);
    free(d);
  }
  else if (eina_list_data_get(list) != (void *)-1)
  {
    d->cb_func(d->data, list);
    free(d);
  }
}

static int
cb_access_point(void *data, E_NM_Access_Point *ap)
{
  Reply_Data  *d;
  Eina_List   *list;

  d = data;
  list = d->reply;
  if (ap)
    list = eina_list_append(list, ap);
  list = eina_list_remove_list(list, list);

  check_done(d, list);
  d->reply = list;
  return 1;
}

static void
cb_access_points(void *data, void *reply, DBusError *err)
{
  Reply_Data           *d;
  E_NM_Device_Internal *dev;
  Eina_List            *access_points;
  Eina_List            *list = NULL;
  Eina_List            *l;
  const char           *ap;

  d = data;
  dev = d->object;
  if (dbus_error_is_set(err))
  {
    ERR("%s - %s", err->name, err->message);
    d->cb_func(d->data, NULL);
    free(d);
    return;
  }
  access_points = reply;
  //TODO: ecore_list_free_cb_set(list, ECORE_FREE_CB(e_nm_access_point_free));
  EINA_LIST_FOREACH(access_points, l, ap)
  {
    list = eina_list_prepend(list, (void *)-1);
    d->reply = list;
    e_nm_access_point_get(&(dev->nmi->nm), ap, cb_access_point, d);
  }
}

EAPI int
e_nm_device_get(E_NM *nm, const char *device,
                int (*cb_func)(void *data, E_NM_Device *device),
                void *data)
{
  E_NM_Internal *nmi;
  E_NM_Device_Internal *dev;
  Property_Data     *d;

  nmi = (E_NM_Internal *)nm;
  dev = calloc(1, sizeof(E_NM_Device_Internal));
  dev->nmi = nmi;
  d = calloc(1, sizeof(Property_Data));
  d->nmi = nmi;
  d->cb_func = OBJECT_CB(cb_func);
  d->data = data;
  d->reply = dev;
  d->property = device_properties;
  d->service = E_NM_SERVICE;
  d->object = strdup(device);
  d->interface = E_NM_INTERFACE_DEVICE;

  dev->handlers = eina_list_append(dev->handlers, e_nm_device_signal_handler_add(nmi->conn, device, "StateChanged", cb_state_changed, dev));
  dev->handlers = eina_list_append(dev->handlers, e_nm_device_signal_handler_add(nmi->conn, device, "PropertiesChanged", cb_properties_changed, dev));
 
  return property_get(nmi->conn, d);
}

EAPI void
e_nm_device_free(E_NM_Device *device)
{
  E_NM_Device_Internal *dev;
  void *data;

  if (!device) return;
  dev = (E_NM_Device_Internal *)device;
  if (dev->dev.udi) free(dev->dev.udi);
  if (dev->dev.interface) free(dev->dev.interface);
  if (dev->dev.driver) free(dev->dev.driver);
  if (dev->dev.ip4_config) free(dev->dev.ip4_config);
  if (dev->dev.dhcp4_config) free(dev->dev.dhcp4_config);
  switch (dev->dev.device_type)
  {
    case E_NM_DEVICE_TYPE_WIRED:
      if (dev->dev.wired.hw_address) free(dev->dev.wired.hw_address);
      break;
    case E_NM_DEVICE_TYPE_WIRELESS:
      if (dev->dev.wireless.hw_address) free(dev->dev.wireless.hw_address);
      if (dev->dev.wireless.active_access_point) free(dev->dev.wireless.active_access_point);
      break;
  }
  EINA_LIST_FREE(dev->handlers, data)
    e_dbus_signal_handler_del(dev->nmi->conn, data);
  free(dev);
}

EAPI void
e_nm_device_dump(E_NM_Device *dev)
{
  if (!dev) return;
  printf("E_NM_Device:\n");
  printf("udi                  : %s\n", dev->udi);
  printf("interface            : %s\n", dev->interface);
  printf("driver               : %s\n", dev->driver);
  printf("capabilities         : ");
  if (dev->capabilities & E_NM_DEVICE_CAP_NM_SUPPORTED)
    printf(" E_NM_DEVICE_CAP_NM_SUPPORTED");
  if (dev->capabilities & E_NM_DEVICE_CAP_CARRIER_DETECT)
    printf(" E_NM_DEVICE_CAP_CARRIER_DETECT");
  if (dev->capabilities == E_NM_DEVICE_CAP_NONE)
    printf(" E_NM_DEVICE_CAP_NONE");
  printf("\n");
  printf("ip4_address          : %s\n", ip4_address2str(dev->ip4_address));
  printf("state                : ");
  switch (dev->state)
  {
    case E_NM_DEVICE_STATE_UNKNOWN:
      printf("E_NM_DEVICE_STATE_UNKNOWN\n");
      break;
    case E_NM_DEVICE_STATE_UNMANAGED:
      printf("E_NM_DEVICE_STATE_UNMANAGED\n");
      break;
    case E_NM_DEVICE_STATE_UNAVAILABLE:
      printf("E_NM_DEVICE_STATE_UNAVAILABLE\n");
      break;
    case E_NM_DEVICE_STATE_DISCONNECTED:
      printf("E_NM_DEVICE_STATE_DISCONNECTED\n");
      break;
    case E_NM_DEVICE_STATE_PREPARE:
      printf("E_NM_DEVICE_STATE_PREPARE\n");
      break;
    case E_NM_DEVICE_STATE_CONFIG:
      printf("E_NM_DEVICE_STATE_CONFIG\n");
      break;
    case E_NM_DEVICE_STATE_NEED_AUTH:
      printf("E_NM_DEVICE_STATE_NEED_AUTH\n");
      break;
    case E_NM_DEVICE_STATE_IP_CONFIG:
      printf("E_NM_DEVICE_STATE_IP_CONFIG\n");
      break;
    case E_NM_DEVICE_STATE_ACTIVATED:
      printf("E_NM_DEVICE_STATE_ACTIVATED\n");
      break;
    case E_NM_DEVICE_STATE_FAILED:
      printf("E_NM_DEVICE_STATE_FAILED\n");
      break;
  }
  printf("ip4_config           : %s\n", dev->ip4_config);
  printf("dhcp4_config         : %s\n", dev->dhcp4_config);
  printf("managed              : %d\n", dev->managed);
  printf("device_type          : %u\n", dev->device_type);
  switch (dev->device_type)
  {
    case E_NM_DEVICE_TYPE_WIRED:
      printf("hw_address           : %s\n", dev->wired.hw_address);
      printf("speed                : %u\n", dev->wired.speed);
      printf("carrier              : %d\n", dev->wired.carrier);
      break;
    case E_NM_DEVICE_TYPE_WIRELESS:
      printf("hw_address           : %s\n", dev->wireless.hw_address);
      printf("mode                 : ");
      switch (dev->wireless.mode)
      {
        case E_NM_802_11_MODE_UNKNOWN:
          printf("E_NM_802_11_MODE_UNKNOWN\n");
          break;
        case E_NM_802_11_MODE_ADHOC:
          printf("E_NM_802_11_MODE_ADHOC\n");
          break;
        case E_NM_802_11_MODE_INFRA:
          printf("E_NM_802_11_MODE_INFRA\n");
          break;
      }
      printf("bitrate              : %u\n", dev->wireless.bitrate);
      printf("active_access_point  : %s\n", dev->wireless.active_access_point);
      printf("wireless_capabilities: ");
      if (dev->wireless.wireless_capabilities & E_NM_802_11_DEVICE_CAP_CIPHER_WEP40)
        printf(" E_NM_802_11_DEVICE_CAP_CIPHER_WEP40");
      if (dev->wireless.wireless_capabilities & E_NM_802_11_DEVICE_CAP_CIPHER_WEP104)
        printf(" E_NM_802_11_DEVICE_CAP_CIPHER_WEP104");
      if (dev->wireless.wireless_capabilities & E_NM_802_11_DEVICE_CAP_CIPHER_TKIP)
        printf(" E_NM_802_11_DEVICE_CAP_CIPHER_TKIP");
      if (dev->wireless.wireless_capabilities & E_NM_802_11_DEVICE_CAP_CIPHER_CCMP)
        printf(" E_NM_802_11_DEVICE_CAP_CIPHER_CCMP");
      if (dev->wireless.wireless_capabilities & E_NM_802_11_DEVICE_CAP_WPA)
        printf(" E_NM_802_11_DEVICE_CAP_WPA");
      if (dev->wireless.wireless_capabilities & E_NM_802_11_DEVICE_CAP_RSN)
        printf(" E_NM_802_11_DEVICE_CAP_RSN");
      if (dev->wireless.wireless_capabilities == E_NM_802_11_DEVICE_CAP_NONE)
        printf(" E_NM_802_11_DEVICE_CAP_NONE");
      printf("\n");
      break;
  }
}

EAPI int
e_nm_device_wireless_get_access_points(E_NM_Device *device, int (*cb_func)(void *data, Eina_List *access_points), void *data)
{
  DBusMessage          *msg;
  Reply_Data           *d;
  E_NM_Device_Internal *dev;
  int                   ret;

  if (device->device_type != E_NM_DEVICE_TYPE_WIRELESS) return 0;

  dev = (E_NM_Device_Internal *)device;
  d = calloc(1, sizeof(Reply_Data));
  d->object = dev;
  d->cb_func = OBJECT_CB(cb_func);
  d->data = data;

  msg = e_nm_device_wireless_call_new(dev->dev.udi, "GetAccessPoints");

  ret = e_dbus_method_call_send(dev->nmi->conn, msg, cb_nm_object_path_list, cb_access_points, free_nm_object_path_list, -1, d) ? 1 : 0;
  dbus_message_unref(msg);
  return ret;
}

EAPI void
e_nm_device_data_set(E_NM_Device *device, void *data)
{
  E_NM_Device_Internal *dev;

  dev = (E_NM_Device_Internal *)device;
  dev->data = data;
}

EAPI void *
e_nm_device_data_get(E_NM_Device *device)
{
  E_NM_Device_Internal *dev;

  dev = (E_NM_Device_Internal *)device;
  return dev->data;
}

EAPI void
e_nm_device_callback_state_changed_set(E_NM_Device *device, int (*cb_func)(E_NM_Device *device, E_NM_State new_state, E_NM_State old_state, E_NM_Device_State_Reason reason))
{
  E_NM_Device_Internal *dev;

  dev = (E_NM_Device_Internal *)device;
  dev->state_changed = cb_func;
}

EAPI void
e_nm_device_callback_properties_changed_set(E_NM_Device *device, int (*cb_func)(E_NM_Device *dev))
{
  E_NM_Device_Internal *dev;

  dev = (E_NM_Device_Internal *)device;
  dev->properties_changed = cb_func;
}

EAPI void
e_nm_device_wireless_callback_access_point_added_set(E_NM_Device *device, int (*cb_func)(E_NM_Device *device, const char *access_point))
{
  E_NM_Device_Internal *dev;

  if (device->device_type != E_NM_DEVICE_TYPE_WIRELESS) return;
  dev = (E_NM_Device_Internal *)device;
  dev->access_point_added = cb_func;
}

EAPI void
e_nm_device_wireless_callback_access_point_removed_set(E_NM_Device *device, int (*cb_func)(E_NM_Device *device, const char *access_point))
{
  E_NM_Device_Internal *dev;

  if (device->device_type != E_NM_DEVICE_TYPE_WIRELESS) return;
  dev = (E_NM_Device_Internal *)device;
  dev->access_point_removed = cb_func;
}

