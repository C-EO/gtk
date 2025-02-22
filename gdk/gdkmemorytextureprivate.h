/*
 * Copyright © 2018 Benjamin Otte
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Benjamin Otte <otte@gnome.org>
 */

#ifndef __GDK_MEMORY_TEXTURE_PRIVATE_H__
#define __GDK_MEMORY_TEXTURE_PRIVATE_H__

#include "gdkmemorytexture.h"

#include "gdktextureprivate.h"

G_BEGIN_DECLS

#define GDK_MEMORY_GDK_PIXBUF_OPAQUE GDK_MEMORY_R8G8B8
#define GDK_MEMORY_GDK_PIXBUF_ALPHA GDK_MEMORY_R8G8B8A8

typedef enum {
  GDK_MEMORY_CONVERT_DOWNLOAD_LITTLE_ENDIAN,
  GDK_MEMORY_CONVERT_DOWNLOAD_BIT_ENDIAN,
  GDK_MEMORY_CONVERT_GLES_RGBA,

  GDK_MEMORY_N_CONVERSIONS
} GdkMemoryConversion;

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
#define GDK_MEMORY_CONVERT_DOWNLOAD GDK_MEMORY_CONVERT_DOWNLOAD_LITTLE_ENDIAN
#elif G_BYTE_ORDER == G_BIG_ENDIAN
#define GDK_MEMORY_CONVERT_DOWNLOAD GDK_MEMORY_CONVERT_DOWNLOAD_BIG_ENDIAN
#else
#error "Unknown byte order for GDK_MEMORY_CONVERT_DOWNLOAD"
#endif

gsize                   gdk_memory_format_bytes_per_pixel   (GdkMemoryFormat    format);

GdkMemoryFormat         gdk_memory_texture_get_format       (GdkMemoryTexture  *self);
const guchar *          gdk_memory_texture_get_data         (GdkMemoryTexture  *self);
gsize                   gdk_memory_texture_get_stride       (GdkMemoryTexture  *self);

void                    gdk_memory_convert                  (guchar                     *dest_data,
                                                             gsize                       dest_stride,
                                                             GdkMemoryConversion         dest_format,
                                                             const guchar               *src_data,
                                                             gsize                       src_stride,
                                                             GdkMemoryFormat             src_format,
                                                             gsize                       width,
                                                             gsize                       height);

void                    gdk_memory_convert_to_float         (float                      *dest_data,
                                                             gsize                       dest_stride,
                                                             const guchar               *src_data,
                                                             gsize                       src_stride,
                                                             GdkMemoryFormat             src_format,
                                                             gsize                       width,
                                                             gsize                       height);


G_END_DECLS

#endif /* __GDK_MEMORY_TEXTURE_PRIVATE_H__ */
