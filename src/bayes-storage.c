/* bayes-storage.c
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

#include "bayes-storage.h"

/**
 * SECTION:bayes-storage
 * @title: BayesStorage
 * @short_description: Interface for storing training data.
 *
 * The #BayesStorage interface provides a common interface for storing
 * training data for the classifier.
 *
 * See #BayesStorageMemory for in memory storage of training data.
 */

G_DEFINE_INTERFACE (BayesStorage, bayes_storage, G_TYPE_OBJECT)

static void
bayes_storage_real_add_token_count (BayesStorage *self,
                                    const gchar  *name,
                                    const gchar  *token,
                                    guint         count)
{
}

static gchar **
bayes_storage_real_get_names (BayesStorage *self)
{
  return NULL;
}

static guint
bayes_storage_real_get_token_count (BayesStorage *self,
                                    const gchar  *name,
                                    const gchar  *token)
{
  return 0;
}

static gdouble
bayes_storage_real_get_token_probability (BayesStorage *self,
                                          const gchar  *name,
                                          const gchar  *token)
{
  return 0.0;
}

static void
bayes_storage_default_init (BayesStorageInterface *iface)
{
  iface->add_token_count = bayes_storage_real_add_token_count;
  iface->get_names = bayes_storage_real_get_names;
  iface->get_token_count = bayes_storage_real_get_token_count;
  iface->get_token_probability = bayes_storage_real_get_token_probability;
}

/**
 * bayes_storage_add_token_count:
 * @self: A #BayesStorage.
 * @name: The classification to store the token in.
 * @token: The token to add.
 * @count: The count of times @token was found.
 *
 * This function will add a token to the storage, applying it to the given
 * classification. @count should indiciate the number of times that the
 * token was found.
 */
void
bayes_storage_add_token_count (BayesStorage *self,
                               const gchar  *name,
                               const gchar  *token,
                               guint         count)
{
  g_return_if_fail (BAYES_IS_STORAGE (self));
  g_return_if_fail (name);
  g_return_if_fail (token);
  g_return_if_fail (count);

  BAYES_STORAGE_GET_IFACE (self)->add_token_count (self, name, token, count);
}

/**
 * bayes_storage_add_token:
 * @self: A #BayesStorage.
 * @name: The classification to store the token in.
 * @token: The token to add.
 *
 * This function will add a token to the storage, applying it to the given
 * classification.
 */
void
bayes_storage_add_token (BayesStorage *self,
                         const gchar  *name,
                         const gchar  *token)
{
  g_return_if_fail (BAYES_IS_STORAGE (self));
  g_return_if_fail (name);
  g_return_if_fail (token);

  BAYES_STORAGE_GET_IFACE (self)->add_token_count (self, name, token, 1);
}

/**
 * bayes_storage_get_names:
 * @self: A #BayesStorage.
 *
 * Retrieves the names of the classifications trained in this storage
 * instance. The result should be freed with g_strfreev().
 *
 * Returns: (transfer full): A #GStrv of class names.
 */
gchar **
bayes_storage_get_names (BayesStorage *self)
{
  g_return_val_if_fail (BAYES_IS_STORAGE (self), NULL);

  return BAYES_STORAGE_GET_IFACE (self)->get_names (self);
}

/**
 * bayes_storage_get_token_count:
 * @storage: A #BayesStorage.
 * @name: (nullable): The classification or %NULL for all.
 * @token: (nullable): The token or %NULL for all.
 *
 * Retrieves the number of times @token has been found in the training
 * data. If @token is %NULL, the count of all items in the classification
 * will be retrieved. If @name is %NULL, then the count of all the
 * instances of @token in the all the classifications.
 *
 * Returns: A #guint containing the count of all items.
 */
guint
bayes_storage_get_token_count (BayesStorage *self,
                               const gchar  *name,
                               const gchar  *token)
{
  g_return_val_if_fail (BAYES_IS_STORAGE (self), 0);
  g_return_val_if_fail (name || token, 0);

  return BAYES_STORAGE_GET_IFACE (self)->get_token_count (self, name, token);
}

/**
 * bayes_storage_get_token_probability:
 * @storage: A #BayesStorage.
 * @name: The classification.
 * @token: The desired token.
 *
 * Checks to see the probability of a token being a given classification.
 *
 * Returns: A #gdouble between 0.0 and 1.0 containing the probability.
 */
gdouble
bayes_storage_get_token_probability (BayesStorage *self,
                                     const gchar  *name,
                                     const gchar  *token)
{
  g_return_val_if_fail (BAYES_IS_STORAGE (self), 0.0);
  g_return_val_if_fail (name, 0.0);
  g_return_val_if_fail (token, 0.0);

  return BAYES_STORAGE_GET_IFACE (self)->get_token_probability (self, name, token);
}
