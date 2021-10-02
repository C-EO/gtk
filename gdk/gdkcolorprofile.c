/* gdkcolor_profile.c
 *
 * Copyright 2021 (c) Benjamin Otte
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
 */

/**
 * GdkColorProfile:
 *
 * `GdkColorProfile` is used to describe color spaces.
 *
 * It is used to associate color profiles defined by the [International
 * Color Consortium (ICC)](https://color.org/) with texture and color data.
 *
 * Each `GdkColorProfile` encapsulates an
 * [ICC profile](https://en.wikipedia.org/wiki/ICC_profile). That profile
 * can be queried via the using [property@Gdk.ColorProfile:icc-profile]
 * property.
 *
 * `GdkColorProfile` objects are immutable and therefore threadsafe.
 *
 * Since: 4.6
 */

#include "config.h"

#include "gdkcolorprofileprivate.h"

#include "gdkintl.h"

struct _GdkColorProfile
{
  GObject parent_instance;

  GBytes *icc_profile;
  cmsHPROFILE lcms_profile;
};

struct _GdkColorProfileClass
{
  GObjectClass parent_class;
};

enum {
  PROP_0,
  PROP_ICC_PROFILE,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];

static gboolean
gdk_color_profile_real_init (GInitable     *initable,
                             GCancellable  *cancellable,
                             GError       **error)
{
  GdkColorProfile *self = GDK_COLOR_PROFILE (initable);

  if (self->lcms_profile == NULL)
    {
      const guchar *data;
      gsize size;

      if (self->icc_profile == NULL)
        self->icc_profile = g_bytes_new (NULL, 0);

      data = g_bytes_get_data (self->icc_profile, &size);

      self->lcms_profile = cmsOpenProfileFromMem (data, size);
      if (self->lcms_profile == NULL)
        {
          g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Failed to load ICC profile"));
          return FALSE;
        }
    }

  return TRUE;
}

static void
gdk_color_profile_initable_init (GInitableIface *iface)
{
  iface->init = gdk_color_profile_real_init;
}


G_DEFINE_TYPE_WITH_CODE (GdkColorProfile, gdk_color_profile, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, gdk_color_profile_initable_init))

static void
gdk_color_profile_set_property (GObject      *gobject,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  GdkColorProfile *self = GDK_COLOR_PROFILE (gobject);

  switch (prop_id)
    {
    case PROP_ICC_PROFILE:
      self->icc_profile = g_value_dup_boxed (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
gdk_color_profile_get_property (GObject    *gobject,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  GdkColorProfile *self = GDK_COLOR_PROFILE (gobject);

  switch (prop_id)
    {
    case PROP_ICC_PROFILE:
      g_value_set_boxed (value, self->icc_profile);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
gdk_color_profile_dispose (GObject *object)
{
  GdkColorProfile *self = GDK_COLOR_PROFILE (object);

  g_clear_pointer (&self->icc_profile, g_bytes_unref);
  g_clear_pointer (&self->lcms_profile, cmsCloseProfile);

  G_OBJECT_CLASS (gdk_color_profile_parent_class)->dispose (object);
}

static void
gdk_color_profile_class_init (GdkColorProfileClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = gdk_color_profile_set_property;
  gobject_class->get_property = gdk_color_profile_get_property;
  gobject_class->dispose = gdk_color_profile_dispose;

  /**
   * GdkColorProfile:icc-profile: (attributes org.gtk.Property.get=gdk_color_profile_get_icc_profile)
   *
   * the ICC profile for this color profile
   */
  properties[PROP_ICC_PROFILE] =
    g_param_spec_boxed ("icc-profile",
                        P_("ICC profile"),
                        P_("ICC profile for this color profile"),
                        G_TYPE_BYTES,
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT_ONLY |
                        G_PARAM_STATIC_STRINGS |
                        G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (gobject_class, N_PROPS, properties);
}

static void
gdk_color_profile_init (GdkColorProfile *self)
{
}

/**
 * gdk_color_profile_get_srgb:
 *
 * Returns the color profile representing the sRGB color space.
 *
 * If you don't know anything about color profiles but need one for
 * use with some function, this one is most likely the right one.
 *
 * Returns: (transfer none): the color profile for the sRGB
 *   color space.
 *
 * Since: 4.6
 */
GdkColorProfile *
gdk_color_profile_get_srgb (void)
{
  static GdkColorProfile *srgb_profile;

  if (g_once_init_enter (&srgb_profile))
    {
      GdkColorProfile *new_profile;

      new_profile = gdk_color_profile_new_from_lcms_profile (cmsCreate_sRGBProfile (), NULL);
      g_assert (new_profile);

      g_once_init_leave (&srgb_profile, new_profile);
    }

  return srgb_profile;
}

/**
 * gdk_color_profile_new_from_icc_bytes:
 * @icc_profile: The ICC profiles given as a `GBytes`
 * @error: Return location for an error
 *
 * Creates a new color profile for the given ICC profile data.
 *
 * if the profile is not valid, %NULL is returned and an error
 * is raised.
 *
 * Returns: a new `GdkColorProfile` or %NULL on error
 *
 * Since: 4.6
 */
GdkColorProfile *
gdk_color_profile_new_from_icc_bytes (GBytes  *bytes,
                                      GError **error)
{
  g_return_val_if_fail (bytes != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return g_initable_new (GDK_TYPE_COLOR_PROFILE,
                         NULL,
                         error,
                         "icc-profile", bytes,
                         NULL);
}

GdkColorProfile *
gdk_color_profile_new_from_lcms_profile (cmsHPROFILE   lcms_profile,
                                         GError      **error)
{
  GdkColorProfile *result;
  cmsUInt32Number size;
  guchar *data;

  size = 0;
  if (!cmsSaveProfileToMem (lcms_profile, NULL, &size))
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Could not prepare ICC profile"));
      return NULL;
    }

  data = g_malloc (size);
  if (!cmsSaveProfileToMem (lcms_profile, data, &size))
    {
      g_free (data);
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Failed to save ICC profile"));
      return NULL;
    }

  result = g_object_new (GDK_TYPE_COLOR_PROFILE, NULL);
  result->lcms_profile = lcms_profile;
  result->icc_profile = g_bytes_new_take (data, size);

  return result;
}

/**
 * gdk_color_profile_get_icc_profile: (attributes org.gtk.Method.get_property=icc-profile)
 * @self: a `GdkColorProfile`
 *
 * Returns the serialized ICC profile of @self as %GBytes.
 *
 * Returns: (transfer none): the ICC profile
 *
 * Since: 4.6
 */
GBytes *
gdk_color_profile_get_icc_profile (GdkColorProfile *self)
{
  g_return_val_if_fail (GDK_IS_COLOR_PROFILE (self), NULL);

  return self->icc_profile;
}

cmsHPROFILE *
gdk_color_profile_get_lcms_profile (GdkColorProfile *self)
{
  g_return_val_if_fail (GDK_IS_COLOR_PROFILE (self), NULL);

  return self->lcms_profile;
}

/**
 * gdk_color_profile_equal:
 * @profile1: (type GdkColorProfile): a `GdkColorProfile`
 * @profile2: (type GdkColorProfile): another `GdkColorProfile`
 *
 * Compares two `GdkColorProfile`s for equality.
 *
 * Note that this function is not guaranteed to be perfect and two equal
 * profiles may compare not equal. However, different profiles will
 * never compare equal.
 *
 * Returns: %TRUE if the two color profiles compare equal
 *
 * Since: 4.6
 */
gboolean
gdk_color_profile_equal (gconstpointer profile1,
                         gconstpointer profile2)
{
  return profile1 == profile2 ||
         g_bytes_equal (GDK_COLOR_PROFILE (profile1)->icc_profile,
                        GDK_COLOR_PROFILE (profile2)->icc_profile);
}

