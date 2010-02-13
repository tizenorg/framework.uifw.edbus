#include "e_bluez_private.h"
#include <stdlib.h>
#include <string.h>

static E_DBus_Signal_Handler *cb_name_owner_changed = NULL;
static DBusPendingCall *pending_get_name_owner = NULL;
static unsigned int init_count = 0;
static char *unique_name = NULL;

static const char bus_name[] = "org.bluez";
static const char fdo_bus_name[] = "org.freedesktop.DBus";
static const char fdo_interface[] = "org.freedesktop.DBus";
static const char fdo_path[] = "/org/freedesktop/DBus";

E_DBus_Connection *e_bluez_conn = NULL;

EAPI int E_BLUEZ_EVENT_MANAGER_IN = 0;
EAPI int E_BLUEZ_EVENT_MANAGER_OUT = 0;
EAPI int E_BLUEZ_EVENT_ELEMENT_ADD = 0;
EAPI int E_BLUEZ_EVENT_ELEMENT_DEL = 0;
EAPI int E_BLUEZ_EVENT_ELEMENT_UPDATED = 0;

const char *e_bluez_iface_manager = NULL;
const char *e_bluez_iface_adapter = NULL;
const char *e_bluez_iface_device = NULL;

int _e_dbus_bluez_log_dom = -1;

const char *
e_bluez_system_bus_name_get(void)
{
   return unique_name ? unique_name : bus_name;
}

/***********************************************************************
 * Manager
 ***********************************************************************/

/*
 * FIXME: Do we really need to call Manager.GetProperties()?
 */

/**
 * Synchronize elements with server.
 *
 * This will call Manager.GetProperties() on server, retrieve properties
 * and some element paths and then request their properties.
 *
 * This call will add events E_BLUEZ_EVENT_ELEMENT_ADD and
 * E_BLUEZ_EVENT_ELEMENT_UPDATED to the main loop.
 *
 * This will not remove stale elements.
 *
 * @return 1 on success, 0 otherwise.
 */
bool
e_bluez_manager_sync_elements(void)
{
   E_Bluez_Element *manager;

   if (!unique_name)
     return FALSE;
   manager = e_bluez_element_register(manager_path, e_bluez_iface_manager);
   if (manager)
     e_bluez_element_properties_sync(manager);
   else
     return FALSE;

   DBG("sync_manager: %s (%s)", unique_name, bus_name);

   return TRUE;
}

static void
_e_bluez_system_name_owner_exit(void)
{
   e_bluez_manager_clear_elements();
   ecore_event_add(E_BLUEZ_EVENT_MANAGER_OUT, NULL, NULL, NULL);

   free(unique_name);
   unique_name = NULL;
}

static void
_e_bluez_system_name_owner_enter(const char *uid)
{
   DBG("enter bluez at %s (old was %s)", uid, unique_name);
   if (unique_name && strcmp(unique_name, uid) == 0)
     {
	DBG("same unique_name for bluez, ignore.");
	return;
     }

   if (unique_name)
     _e_bluez_system_name_owner_exit();

   unique_name = strdup(uid);

   ecore_event_add(E_BLUEZ_EVENT_MANAGER_IN, NULL, NULL, NULL);
   e_bluez_manager_sync_elements();
}


static void
_e_bluez_system_name_owner_changed(void *data __UNUSED__, DBusMessage *msg)
{
   DBusError err;
   const char *name, *from, *to;

   dbus_error_init(&err);
   if (!dbus_message_get_args(msg, &err,
			      DBUS_TYPE_STRING, &name,
			      DBUS_TYPE_STRING, &from,
			      DBUS_TYPE_STRING, &to,
			      DBUS_TYPE_INVALID))
     {
	ERR("could not get NameOwnerChanged arguments: %s: %s",
	    err.name, err.message);
	dbus_error_free(&err);
	return;
     }

   if (strcmp(name, bus_name) != 0)
     return;

   DBG("NameOwnerChanged from=[%s] to=[%s]", from, to);

   if (from[0] == '\0' && to[0] != '\0')
     _e_bluez_system_name_owner_enter(to);
   else if (from[0] != '\0' && to[0] == '\0')
     {
	DBG("exit bluez at %s", from);
	if (strcmp(unique_name, from) != 0)
	  DBG("%s was not the known name %s, ignored.", from, unique_name);
	else
	  _e_bluez_system_name_owner_exit();
     }
   else
     DBG("unknow change from %s to %s", from, to);
}

static void
_e_bluez_get_name_owner(void *data __UNUSED__, DBusMessage *msg, DBusError *err)
{
   DBusMessageIter itr;
   int t;
   const char *uid;

   pending_get_name_owner = NULL;

   if (!_dbus_callback_check_and_init(msg, &itr, err))
     return;

   t = dbus_message_iter_get_arg_type(&itr);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_STRING))
     return;

   dbus_message_iter_get_basic(&itr, &uid);
   if (!uid)
     {
	ERR("no name owner!");
	return;
     }

   _e_bluez_system_name_owner_enter(uid);
   return;
}

/**
 * Initialize E Connection Manager (E_Connman) system.
 *
 * This will connect and watch org.moblin.bluez.Manager and Element
 * events and translate to Ecore main loop events, also provide a
 * proxy for method invocation on server.
 *
 * Interesting events are:
 *   - E_BLUEZ_EVENT_MANAGER_IN: issued when bluez is avaiable.
 *   - E_BLUEZ_EVENT_MANAGER_OUT: issued when bluez connection is lost.
 *   - E_BLUEZ_EVENT_ELEMENT_ADD: element was added.
 *   - E_BLUEZ_EVENT_ELEMENT_DEL: element was deleted.
 *   - E_BLUEZ_EVENT_ELEMENT_UPDATED: element was updated (properties
 *     or state changed).
 *
 * Manager IN/OUT events do not provide any event information, just
 * tells you that system is usable or not. After manager is out, all
 * elements will be removed, so after this event do not use the system anymore.
 *
 * Element events will give you an element object. After DEL event callback
 * returns, that element will not be valid anymore.
 */
unsigned int
e_bluez_system_init(E_DBus_Connection *edbus_conn)
{
	init_count++;

	if (init_count > 1)
		return init_count;

	_e_dbus_bluez_log_dom = eina_log_domain_register
		("e_dbus_bluez", EINA_LOG_DEFAULT_COLOR);

	if(_e_dbus_bluez_log_dom < 0) {
		EINA_LOG_ERR
			("impossible to create a log domain for edbus_bluez module");
		return -1;
	}

	if (E_BLUEZ_EVENT_MANAGER_IN == 0)
		E_BLUEZ_EVENT_MANAGER_IN = ecore_event_type_new();
	if (E_BLUEZ_EVENT_MANAGER_OUT == 0)
		E_BLUEZ_EVENT_MANAGER_OUT = ecore_event_type_new();
	if (E_BLUEZ_EVENT_ELEMENT_ADD == 0)
		E_BLUEZ_EVENT_ELEMENT_ADD = ecore_event_type_new();
	if (E_BLUEZ_EVENT_ELEMENT_DEL == 0)
		E_BLUEZ_EVENT_ELEMENT_DEL = ecore_event_type_new();
	if (E_BLUEZ_EVENT_ELEMENT_UPDATED == 0)
		E_BLUEZ_EVENT_ELEMENT_UPDATED = ecore_event_type_new();

	if (e_bluez_iface_manager == NULL)
		e_bluez_iface_manager = eina_stringshare_add("org.bluez.Manager");
	if (e_bluez_iface_adapter == NULL)
		e_bluez_iface_adapter = eina_stringshare_add("org.bluez.Adapter");
	if (e_bluez_iface_device == NULL)
		e_bluez_iface_device = eina_stringshare_add("org.bluez.Device");

	e_bluez_conn = edbus_conn;
	cb_name_owner_changed = e_dbus_signal_handler_add
		(e_bluez_conn, fdo_bus_name, fdo_path, fdo_interface, "NameOwnerChanged",
		 _e_bluez_system_name_owner_changed, NULL);

	if (pending_get_name_owner)
		dbus_pending_call_cancel(pending_get_name_owner);

	pending_get_name_owner = e_dbus_get_name_owner
		(e_bluez_conn, bus_name, _e_bluez_get_name_owner, NULL);

	e_bluez_elements_init();

	return init_count;
}


static inline void
_stringshare_del(const char **str)
{
   if (!*str)
     return;
   eina_stringshare_del(*str);
   *str = NULL;
}

/**
 * Shutdown bluez system.
 *
 * When count drops to 0 resources will be released and no calls should be
 * made anymore.
 */
unsigned int
e_bluez_system_shutdown(void)
{
	if (init_count == 0)
	{
		ERR("bluez system already shut down.");
		return 0;
	}
	init_count--;
	if (init_count > 0)
		return init_count;

	_stringshare_del(&e_bluez_iface_manager);
	_stringshare_del(&e_bluez_iface_adapter);
	_stringshare_del(&e_bluez_iface_device);
	return 0;
}
