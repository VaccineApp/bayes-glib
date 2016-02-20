/* bayes-storage.h
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

#ifndef BAYES_STORAGE_H
#define BAYES_STORAGE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define BAYES_TYPE_STORAGE (bayes_storage_get_type())

G_DECLARE_INTERFACE (BayesStorage, bayes_storage, BAYES, STORAGE, GObject)

struct _BayesStorageInterface
{
   GTypeInterface parent;

   void      (*add_token_count)       (BayesStorage *self,
                                       const gchar  *name,
                                       const gchar  *token,
                                       guint         count);
   gchar   **(*get_names)             (BayesStorage *self);
   guint     (*get_token_count)       (BayesStorage *self,
                                       const gchar  *name,
                                       const gchar  *token);
   gdouble   (*get_token_probability) (BayesStorage *self,
                                       const gchar  *name,
                                       const gchar  *token);
};

/**
 * bayes_storage_add_token:
 * @self: A #BayesStorage.
 * @name: The classification to store the token in.
 * @token: The token to add.
 *
 * This function will add a token to the storage, applying it to the given
 * classification.
 */
void      bayes_storage_add_token             (BayesStorage *self,
                                               const gchar  *name,
                                               const gchar  *token);
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
void      bayes_storage_add_token_count       (BayesStorage *self,
                                               const gchar  *name,
                                               const gchar  *token,
                                               guint         count);

/**
 * bayes_storage_get_names:
 * @self: A #BayesStorage.
 *
 * Retrieves the names of the classifications trained in this storage
 * instance. The result should be freed with g_strfreev().
 *
 * Returns: (array zero-terminated=1) (transfer full):
 *      A list of class names.
 */
gchar   **bayes_storage_get_names             (BayesStorage *self);

/**
 * bayes_storage_get_token_count:
 * @self: A #BayesStorage.
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
guint     bayes_storage_get_token_count       (BayesStorage *self,
                                               const gchar  *name,
                                               const gchar  *token);

/**
 * bayes_storage_get_token_probability:
 * @self: A #BayesStorage.
 * @name: The classification.
 * @token: The desired token.
 *
 * Checks to see the probability of a token being a given classification.
 *
 * Returns: A #gdouble between 0.0 and 1.0 containing the probability.
 */
gdouble   bayes_storage_get_token_probability (BayesStorage *self,
                                               const gchar  *name,
                                               const gchar  *token);

G_END_DECLS

#endif /* BAYES_STORAGE_H */
