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

G_BEGIN_DECLS

#define BAYES_TYPE_STORAGE_MEMORY (bayes_storage_memory_get_type())

G_DECLARE_FINAL_TYPE (BayesStorageMemory, bayes_storage_memory, BAYES, STORAGE_MEMORY, GObject)

BayesStorage *bayes_storage_memory_new (void);

G_END_DECLS

#endif /* BAYES_STORAGE_MEMORY_H */
