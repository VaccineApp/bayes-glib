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

void      bayes_storage_add_token             (BayesStorage *self,
                                               const gchar  *name,
                                               const gchar  *token);
void      bayes_storage_add_token_count       (BayesStorage *self,
                                               const gchar  *name,
                                               const gchar  *token,
                                               guint         count);
gchar   **bayes_storage_get_names             (BayesStorage *self);
guint     bayes_storage_get_token_count       (BayesStorage *storage,
                                               const gchar  *name,
                                               const gchar  *token);
gdouble   bayes_storage_get_token_probability (BayesStorage *self,
                                               const gchar  *name,
                                               const gchar  *token);

G_END_DECLS

#endif /* BAYES_STORAGE_H */
