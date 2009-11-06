#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include "E_Nm.h"
#include "e_nm_private.h"
#include "e_dbus_private.h"

static void
cb_updated(void *data, DBusMessage *msg)
{
  E_NMS_Connection_Internal *conn;
  Eina_Hash                 *settings;
  if (!msg || !data) return;

  conn = data;
  settings = parse_settings(msg);

  if (conn->updated)
    conn->updated(&(conn->conn), settings);
}

static void
cb_settings(void *data, DBusMessage *msg, DBusError *err)
{
  Reply_Data *d;
  Eina_Hash  *settings;

  d = data;
  if (dbus_error_is_set(err))
  {
    ERR("%s - %s", err->name, err->message);
    d->cb_func(d->data, NULL);
    free(d);
    return;
  }

  settings = parse_settings(msg);
  d->cb_func(d->data, settings);
  free(d);
}

static void
cb_secrets(void *data, DBusMessage *msg, DBusError *err)
{
  Reply_Data *d;
  Eina_Hash  *secrets;

  d = data;
  if (dbus_error_is_set(err))
  {
    ERR("%s - %s", err->name, err->message);
    d->cb_func(d->data, NULL);
    free(d);
    return;
  }

  secrets = parse_settings(msg);
  d->cb_func(d->data, secrets);
  free(d);
}

EAPI E_NMS_Connection *
e_nms_connection_get(E_NMS *nms, const char *service_name, const char *connection)
{
  E_NMS_Internal            *nmsi;
  E_NMS_Connection_Internal *conn;

  nmsi = (E_NMS_Internal *)nms;
  conn = calloc(1, sizeof(E_NMS_Connection_Internal));
  conn->nmi = nmsi->nmi;
  conn->conn.path = strdup(connection);
  conn->conn.service_name = strdup(service_name);
  conn->handlers = eina_list_append(conn->handlers, e_nms_connection_signal_handler_add(nmsi->nmi->conn, service_name, connection, "Updated", cb_updated, conn));

  return &conn->conn;
}

EAPI void
e_nms_connection_free(E_NMS_Connection *connection)
{
  E_NMS_Connection_Internal *conn;
  void *data;

  if (!connection) return;
  conn = (E_NMS_Connection_Internal *)connection;

  if (conn->conn.service_name) free(conn->conn.service_name);
  if (conn->conn.path) free(conn->conn.path);
  EINA_LIST_FREE(conn->handlers, data)
    e_dbus_signal_handler_del(conn->nmi->conn, data);
  free(conn);
}

EAPI void
e_nms_connection_dump(E_NMS_Connection *conn)
{
  if (!conn) return;

  printf("E_NMS_Connection:\n");
  printf("service_name: %s\n", conn->service_name);
  printf("path        : %s\n", conn->path);
}

EAPI int
e_nms_connection_get_settings(E_NMS_Connection *connection, int (*cb_func)(void *data, Eina_Hash *settings), void *data)
{
  DBusMessage *msg;
  Reply_Data   *d;
  E_NMS_Connection_Internal *conn;
  int ret;

  conn = (E_NMS_Connection_Internal *)connection;
  d = calloc(1, sizeof(Reply_Data));
  d->object = conn;
  d->cb_func = OBJECT_CB(cb_func);
  d->data = data;

  msg = e_nms_connection_call_new(conn->conn.service_name, conn->conn.path, "GetSettings");

  ret = e_dbus_message_send(conn->nmi->conn, msg, cb_settings, -1, d) ? 1 : 0;
  dbus_message_unref(msg);
  return ret;
}

EAPI int
e_nms_connection_secrets_get_secrets(E_NMS_Connection *connection, const char *setting_name, Eina_List *hints, int request_new, int (*cb_func)(void *data, Eina_Hash *secrets), void *data)
{
  DBusMessage      *msg;
  DBusMessageIter   iter, a_iter;
  Reply_Data       *d;
  E_NMS_Connection_Internal *conn;
  int ret;
  const char *hint;
  Eina_List *l;

  conn = (E_NMS_Connection_Internal *)connection;
  d = calloc(1, sizeof(Reply_Data));
  d->object = conn;
  d->cb_func = OBJECT_CB(cb_func);
  d->data = data;

  msg = e_nms_connection_secrets_call_new(conn->conn.service_name, conn->conn.path, "GetSecrets");
  dbus_message_iter_init_append(msg, &iter);
  dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &setting_name);
  dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "s", &a_iter);
  EINA_LIST_FOREACH(hints, l, hint)
    dbus_message_iter_append_basic(&a_iter, DBUS_TYPE_STRING, &hint);
  dbus_message_iter_close_container(&iter, &a_iter);
  dbus_message_iter_append_basic(&iter, DBUS_TYPE_BOOLEAN, &request_new);

  ret = e_dbus_message_send(conn->nmi->conn, msg, cb_secrets, -1, d) ? 1 : 0;
  dbus_message_unref(msg);
  return ret;
}

EAPI void
e_nms_connection_callback_updated_set(E_NMS_Connection *connection, int (*cb_func)(E_NMS_Connection *conn, Eina_Hash *settings))
{
  E_NMS_Connection_Internal *conn;

  conn = (E_NMS_Connection_Internal*)connection;
  conn->updated = cb_func;
}

