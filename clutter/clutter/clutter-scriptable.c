/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Matthew Allum  <mallum@openedhand.com>
 *             Emmanuele Bassi  <ebassi@openedhand.com>
 *
 * Copyright (C) 2006 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

/**
 * ClutterScriptable:
 * 
 * Override the UI definition parsing
 *
 * The #ClutterScriptable interface exposes the UI definition parsing
 * process to external classes. By implementing this interface, a class can
 * override the UI definition parsing and transform complex data types into
 * [class@GObject.Object] properties, or allow custom properties.
 */

#include "clutter/clutter-build-config.h"

#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include "clutter/clutter-scriptable.h"
#include "clutter/clutter-script-private.h"

#include "clutter/clutter-private.h"
#include "clutter/clutter-debug.h"

typedef ClutterScriptableIface  ClutterScriptableInterface;

G_DEFINE_INTERFACE (ClutterScriptable, clutter_scriptable, G_TYPE_OBJECT);

static void
clutter_scriptable_default_init (ClutterScriptableInterface *iface)
{
}

/**
 * clutter_scriptable_set_id:
 * @scriptable: a #ClutterScriptable
 * @id_: the #ClutterScript id of the object
 *
 * Sets @id_ as the unique Clutter script it for this instance of
 * #ClutterScriptableIface.
 *
 * This name can be used by user interface designer applications to
 * define a unique name for an object constructable using the UI
 * definition language parsed by [class@Script].
 */
void
clutter_scriptable_set_id (ClutterScriptable *scriptable,
                           const gchar       *id_)
{
  ClutterScriptableIface *iface;

  g_return_if_fail (CLUTTER_IS_SCRIPTABLE (scriptable));
  g_return_if_fail (id_ != NULL);

  iface = CLUTTER_SCRIPTABLE_GET_IFACE (scriptable);
  if (iface->set_id)
    iface->set_id (scriptable, id_);
  else
    g_object_set_data_full (G_OBJECT (scriptable),
                            "clutter-script-id",
                            g_strdup (id_),
                            g_free);
}

/**
 * clutter_scriptable_get_id:
 * @scriptable: a #ClutterScriptable
 *
 * Retrieves the id of @scriptable set using [method@Clutter.Scriptable.set_id].
 *
 * Return value: the id of the object. The returned string is owned by
 *   the scriptable object and should never be modified of freed
 */
const gchar *
clutter_scriptable_get_id (ClutterScriptable *scriptable)
{
  ClutterScriptableIface *iface;

  g_return_val_if_fail (CLUTTER_IS_SCRIPTABLE (scriptable), NULL);

  iface = CLUTTER_SCRIPTABLE_GET_IFACE (scriptable);
  if (iface->get_id)
    return iface->get_id (scriptable);
  else
    return g_object_get_data (G_OBJECT (scriptable), "clutter-script-id");
}

/**
 * clutter_scriptable_parse_custom_node:
 * @scriptable: a #ClutterScriptable
 * @script: the #ClutterScript creating the scriptable instance
 * @value: the generic value to be set
 * @name: the name of the node
 * @node: the JSON node to be parsed
 *
 * Parses the passed JSON node. The implementation must set the type
 * of the passed [struct@GObject.Value] pointer using g_value_init().
 *
 * Return value: %TRUE if the node was successfully parsed, %FALSE otherwise.
 */
gboolean
clutter_scriptable_parse_custom_node (ClutterScriptable *scriptable,
                                      ClutterScript     *script,
                                      GValue            *value,
                                      const gchar       *name,
                                      JsonNode          *node)
{
  ClutterScriptableIface *iface;

  g_return_val_if_fail (CLUTTER_IS_SCRIPTABLE (scriptable), FALSE);
  g_return_val_if_fail (CLUTTER_IS_SCRIPT (script), FALSE);
  g_return_val_if_fail (name != NULL, FALSE);
  g_return_val_if_fail (node != NULL, FALSE);

  iface = CLUTTER_SCRIPTABLE_GET_IFACE (scriptable);
  if (iface->parse_custom_node)
    return iface->parse_custom_node (scriptable, script, value, name, node);

  return FALSE;
}

/**
 * clutter_scriptable_set_custom_property:
 * @scriptable: a #ClutterScriptable
 * @script: the #ClutterScript creating the scriptable instance
 * @name: the name of the property
 * @value: the value of the property
 *
 * Overrides the common properties setting. The underlying virtual
 * function should be used when implementing custom properties.
 */
void
clutter_scriptable_set_custom_property (ClutterScriptable *scriptable,
                                        ClutterScript     *script,
                                        const gchar       *name,
                                        const GValue      *value)
{
  ClutterScriptableIface *iface;

  g_return_if_fail (CLUTTER_IS_SCRIPTABLE (scriptable));
  g_return_if_fail (CLUTTER_IS_SCRIPT (script));
  g_return_if_fail (name != NULL);
  g_return_if_fail (value != NULL);

  iface = CLUTTER_SCRIPTABLE_GET_IFACE (scriptable);
  if (iface->set_custom_property)
    iface->set_custom_property (scriptable, script, name, value);
}
