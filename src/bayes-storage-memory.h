/* bayes-storage-memory.h
 *
 * Copyright (C) 2012 Christian Hergert <chris@dronelabs.com>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BAYES_STORAGE_MEMORY_H
#define BAYES_STORAGE_MEMORY_H

#include "bayes-storage.h"
#include <gio/gio.h>

G_BEGIN_DECLS

#define BAYES_TYPE_TOKENS (bayes_tokens_get_type ())

typedef struct _BayesTokens BayesTokens;

#define BAYES_TYPE_STORAGE_MEMORY (bayes_storage_memory_get_type())

G_DECLARE_FINAL_TYPE (BayesStorageMemory, bayes_storage_memory, BAYES, STORAGE_MEMORY, GObject)

struct _BayesStorageMemory
{
  GObject     parent_instance;

  /*< private >*/

  /* somehow this is needed to get proper GI types for properties */
  /**
   * BayesStorageMemory:names: (type GLib.HashTable(utf8,Bayes.Tokens))
   */
  GHashTable  *names;

  /**
   * BayesStorageMemory:corpus: (type Bayes.Tokens)
   */
  BayesTokens *corpus;
};

/**
 * bayes_storage_memory_new:
 *
 * Creates a new #BayesStorageMemory instance.
 *
 * Returns: (transfer full): A new #BayesStorageMemory
 */
BayesStorageMemory *bayes_storage_memory_new (void);

/**
 * bayes_storage_memory_new_from_file:
 * @filename: Name of filename to load
 * @error: (allow-none): Return location for an error, or %NULL
 *
 * Creates a new #BayesStorageMemory instance from a file. The
 * file must be a serialized #BayesStorageMemory in JSON format.
 *
 * Returns: (transfer full): a new BayesStorageMemory or %NULL
 * if parsing failed or file could not be loaded.
 */
BayesStorageMemory *bayes_storage_memory_new_from_file (const gchar *filename,
							  GError **error);

/**
 * bayes_storage_memory_new_from_stream:
 * @stream: stream to load from
 * @cancellable: (allow-none): to cancel loading
 * @error: (allow-none): Return location for an error, or %NULL
 *
 * Creates a new #BayesStorageMemory instance from a stream. The
 * stream must be a serialized #BayesStorageMemory in JSON format.
 *
 * Returns: (transfer full): a new #BayesStorageMemory or %NULL
 * if parsing failed or stream could not be read from.
 */
BayesStorageMemory *bayes_storage_memory_new_from_stream (GInputStream *stream,
							    GCancellable *cancellable,
							    GError **error);

/**
 * bayes_storage_memory_save_to_file:
 * @self: a #BayesStorageMemory
 * @filename: name of file to save
 * @error: (allow-none): Return location for an error, or %NULL
 *
 * Serializes a #BayesStorageMemory instance to a file 
 * in JSON format.
 *
 * Returns: %FALSE if @error is set
 */
gboolean bayes_storage_memory_save_to_file (BayesStorageMemory *self,
					    const gchar *filename,
					    GError **error);

G_END_DECLS

#endif /* BAYES_STORAGE_MEMORY_H */
