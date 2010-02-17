#include "e_bluez_private.h"

/**
 * Get the element manager.
 *
 * @return element pointer if found, NULL otherwise.
 */
E_Bluez_Element *
e_bluez_manager_get(void)
{
   return e_bluez_element_get(manager_path);
}

/**
 *
 * Call method DefaultAdapter() on Manager.
 * TODO: Soon this method will be replaced by ANY adapter inside BlueZ, so we
 * won't need t to call it anymore.
 *
 * @param cb function to call when server replies or some error happens.
 *
 * @return 1 on success, 0 otherwise.
 */
bool
e_bluez_manager_default_adapter(E_DBus_Method_Return_Cb cb)
{
   E_Bluez_Element *element = e_bluez_element_get(manager_path);
   const char name[] = "DefaultAdapter";

   EINA_SAFETY_ON_NULL_RETURN_VAL(element, 0);
   return e_bluez_element_call_full
     (element, name, NULL, &element->_pending.properties_get, cb, NULL);
}
