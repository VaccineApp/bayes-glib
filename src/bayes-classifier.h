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

/**
 * bayes_classifier_get_storage:
 * @self: (in): A #BayesClassifier.
 *
 * Retrieves the #BayesStorage used for tokens by @classifier.
 *
 * Returns: (transfer none): A #BayesStorage.
 */
BayesStorage    *bayes_classifier_get_storage   (BayesClassifier *self);

/**
 * bayes_classifier_guess:
 * @self: (in): A #BayesClassifier.
 * @text: (in): Text to tokenize and guess the classification.
 *
 * Tries to guess the classification of @text by tokenizing @text using
 * the tokenizer provided to bayes_classifier_set_tokenizer()
 * and testing each token against the classifiers training.
 *
 * This method returns a #GList of #BayesGuess instances. It is up to the
 * caller to free the individual #BayesGuess structures as well as the
 * containing #GList.
 *
 * |[<!-- language="C" -->
 * GList *list = bayes_classifier_guess (classifier, "who am i?");
 * g_list_foreach (list, (GFunc)bayes_guess_unref);
 * g_list_free (list);
 * ]|
 *
 * Returns: (element-type BayesGuess) (transfer full): The guesses.
 */
GList           *bayes_classifier_guess         (BayesClassifier *self,
                                                 const gchar     *text);

/**
 * bayes_classifier_new:
 *
 * Create a new instance of #BayesClassifier. The instance should be
 * freed using g_object_unref ().
 *
 * See bayes_classifier_train () for how to train your classifier.
 * See bayes_classifier_guess () for guessing the classification of
 * unknown input.
 *
 * Returns: (transfer full): A newly allocated #BayesClassifier.
 */
BayesClassifier *bayes_classifier_new           (void);

/**
 * bayes_classifier_set_storage:
 * @self: (in): A #BayesClassifier.
 * @storage: (in) (allow-none): A #BayesStorage or %NULL.
 *
 * Sets the storage to use for tokens by the classifier.
 * If @storage is %NULL, then in memory storage will be used.
 */
void             bayes_classifier_set_storage   (BayesClassifier *self,
                                                 BayesStorage    *storage);

/**
 * bayes_classifier_set_tokenizer:
 * @self: (in): A #BayesClassifier.
 * @tokenizer: (in): A #BayesTokenizer.
 * @user_data: User data for @tokenizer.
 * @notify: Destruction notification for @user_data.
 *
 * Sets the tokenizer to use to tokenize input text by @classifer for
 * both training using bayes_classifier_train() and guessing using
 * bayes_classifier_guess().
 */
void             bayes_classifier_set_tokenizer (BayesClassifier *self,
                                                 BayesTokenizer   tokenizer,
                                                 gpointer         user_data,
                                                 GDestroyNotify   notify);

/**
 * bayes_classifier_train:
 * @self: (in): A #BayesClassifier.
 * @name: (in): The classification for @text.
 * @text: (in): Text to tokenize and store for guessing.
 *
 * Tokenizes @text and stores the values under the classification named
 * @name. These are used by bayes_classifier_guess () to determine
 * the classification.
 */
void             bayes_classifier_train         (BayesClassifier *self,
                                                 const gchar     *name,
                                                 const gchar     *text);

G_END_DECLS

#endif /* BAYES_CLASSIFIER_H */
