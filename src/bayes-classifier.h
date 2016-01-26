/* bayes-classifier.h
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

#ifndef BAYES_CLASSIFIER_H
#define BAYES_CLASSIFIER_H

#include <glib-object.h>

#include "bayes-storage.h"
#include "bayes-tokenizer.h"

G_BEGIN_DECLS

#define BAYES_TYPE_CLASSIFIER (bayes_classifier_get_type())

G_DECLARE_FINAL_TYPE (BayesClassifier, bayes_classifier, BAYES, CLASSIFIER, GObject)

BayesStorage    *bayes_classifier_get_storage   (BayesClassifier *self);
GList           *bayes_classifier_guess         (BayesClassifier *self,
                                                 const gchar     *text);
BayesClassifier *bayes_classifier_new           (void);
void             bayes_classifier_set_storage   (BayesClassifier *self,
                                                 BayesStorage    *storage);
void             bayes_classifier_set_tokenizer (BayesClassifier *self,
                                                 BayesTokenizer   tokenizer,
                                                 gpointer         user_data,
                                                 GDestroyNotify   notify);
void             bayes_classifier_train         (BayesClassifier *self,
                                                 const gchar     *name,
                                                 const gchar     *text);

G_END_DECLS

#endif /* BAYES_CLASSIFIER_H */
