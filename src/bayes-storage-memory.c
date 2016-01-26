/* bayes-storage-memory.c
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

#include "bayes-storage-memory.h"

/**
 * SECTION:bayes-storage-memory
 * @title: BayesStorageMemory
 * @short_description: Storage of training data in memory.
 *
 * #BayesStorageMemory is an implementation of #BayesStorage that
 * stores the tokens and their associated counts in memory using
 * #GHashTable. It is mean for smaller data sets and offers no
 * storage of the training data to disk. It is mostly handy for
 * just trying things out.
 */

static void bayes_storage_init (BayesStorageInterface *iface);

typedef struct
{
  GHashTable *tokens;
  guint       count;
} Tokens;

struct _BayesStorageMemory
{
  GObject     parent_instance;
  GHashTable *names;
  Tokens     *corpus;
};

G_DEFINE_TYPE_EXTENDED (BayesStorageMemory,
                        bayes_storage_memory,
                        G_TYPE_OBJECT,
                        0,
                        G_IMPLEMENT_INTERFACE (BAYES_TYPE_STORAGE, bayes_storage_init))

static void
tokens_free (gpointer data)
{
  Tokens *tokens = data;

  if (tokens != NULL)
    {
      g_hash_table_unref (tokens->tokens);
      g_free (tokens);
    }
}

/**
 * bayes_storage_memory_new:
 *
 * Creates a new #BayesStorageMemory instance.
 *
 * Returns: (transfer full): A #BayesStorageMemory.
 */
BayesStorage *
bayes_storage_memory_new (void)
{
  return g_object_new (BAYES_TYPE_STORAGE_MEMORY, NULL);
}

static void
tokens_inc (Tokens      *tokens,
            const gchar *token,
            guint        count)
{
  guint *token_count;

  /*
   * Get the container for the token count or create it if necessary.
   */
  if (!(token_count = g_hash_table_lookup (tokens->tokens, token)))
    {
      token_count = g_new0 (guint, 1);
      g_hash_table_insert (tokens->tokens, g_strdup (token), token_count);
    }

  /*
   * Increment the count of the token.
   */
  *token_count += count;
  tokens->count += count;
}

static void
bayes_storage_memory_add_token_count (BayesStorage *storage,
                                      const gchar  *name,
                                      const gchar  *token,
                                      guint         count)
{
  BayesStorageMemory *self = (BayesStorageMemory *)storage;
  Tokens *tokens;

  g_assert (BAYES_IS_STORAGE_MEMORY (self));
  g_assert (name);
  g_assert (token);

  /*
   * Get the classification hashtable or create it if necessary.
   */
  if (!(tokens = g_hash_table_lookup (self->names, name)))
    {
      tokens = g_new0 (Tokens, 1);
      tokens->tokens = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
      g_hash_table_insert (self->names, g_strdup (name), tokens);
    }

  tokens_inc (tokens, token, count);
  tokens_inc (self->corpus, token, count);
}

static guint
bayes_storage_memory_get_token_count (BayesStorage *storage,
                                      const gchar  *name,
                                      const gchar  *token)
{
  BayesStorageMemory *self = (BayesStorageMemory *)storage;
  Tokens *tokens;
  guint *token_count;

  g_assert (BAYES_IS_STORAGE_MEMORY (self));

  tokens = name ? g_hash_table_lookup(self->names, name) : self->corpus;

  if (tokens != NULL)
    {
      if (!token)
        return tokens->count;
      else if ((token_count = g_hash_table_lookup(tokens->tokens, token)))
        return *token_count;
    }

  return 0;
}

static gdouble
bayes_storage_memory_get_token_probability (BayesStorage *storage,
                                            const gchar  *name,
                                            const gchar  *token)
{
  BayesStorageMemory *self = (BayesStorageMemory *)storage;
  gdouble pool_count;
  gdouble them_count;
  gdouble tot_count;
  gdouble this_count;
  gdouble other_count;
  gdouble good_metric;
  gdouble bad_metric;
  gdouble f;
  Tokens *tokens;

  g_assert (BAYES_IS_STORAGE_MEMORY (self));
  g_assert (name);
  g_assert (token);

  if (!(tokens = g_hash_table_lookup(self->names, name)))
    return 0.0;

  pool_count = tokens->count;
  them_count = MAX (self->corpus->count - pool_count, 1);
  this_count = bayes_storage_memory_get_token_count (storage, name, token);
  tot_count = bayes_storage_memory_get_token_count (storage, NULL, token);
  other_count = tot_count - this_count;
  good_metric = (!pool_count) ? 1.0 : MIN (1.0, other_count / pool_count);
  bad_metric = MIN (1.0, this_count / them_count);
  f = bad_metric / (good_metric + bad_metric);

  if (ABS (f - 0.5) >= 0.1)
     return MAX (0.0001, MIN (0.9999, f));

  return 0.0;
}

static gchar **
bayes_storage_memory_get_names (BayesStorage *storage)
{
  BayesStorageMemory *self = (BayesStorageMemory *)storage;
  GHashTableIter iter;
  GPtrArray *ret;
  gchar *key;

  g_assert (BAYES_IS_STORAGE_MEMORY (self));

  ret = g_ptr_array_new ();
  g_hash_table_iter_init (&iter, self->names);
  while (g_hash_table_iter_next (&iter, (gpointer *)&key, NULL))
    g_ptr_array_add (ret, g_strdup (key));
  g_ptr_array_add (ret, NULL);

  return (gchar **)g_ptr_array_free(ret, FALSE);
}

static void
bayes_storage_memory_finalize (GObject *object)
{
  BayesStorageMemory *self = (BayesStorageMemory *)object;

  g_hash_table_unref (self->names);
  tokens_free (self->corpus);

  G_OBJECT_CLASS (bayes_storage_memory_parent_class)->finalize (object);
}

static void
bayes_storage_memory_class_init (BayesStorageMemoryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = bayes_storage_memory_finalize;
}

static void
bayes_storage_memory_init (BayesStorageMemory *self)
{
  Tokens *tokens;

  self->names = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, tokens_free);
  tokens = g_new0(Tokens, 1);
  tokens->tokens = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  self->corpus = tokens;
}

static void
bayes_storage_init (BayesStorageInterface *iface)
{
  iface->add_token_count = bayes_storage_memory_add_token_count;
  iface->get_names = bayes_storage_memory_get_names;
  iface->get_token_count = bayes_storage_memory_get_token_count;
  iface->get_token_probability = bayes_storage_memory_get_token_probability;
}
