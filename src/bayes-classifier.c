/* bayes-classifier.c
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

#include <math.h>

#include "bayes-classifier.h"
#include "bayes-guess.h"
#include "bayes-guess-private.h"
#include "bayes-storage-memory.h"
#include "bayes-tokenizer.h"

/**
 * SECTION:bayes-classifier
 * @title: BayesClassifier
 * @short_description: Bayesian classifier for textual data.
 *
 * #BayesClassifier provides a simple API for classifying textual data.
 * It can be trained using classified input data and then asked to classify
 * input data of unknown classification.
 *
 * The tokenization of the input data can be changed to suit your desired
 * needs. For example, it could be used to train SPAM vs HAM, or perhaps
 * even guess if your boyfriend or girlfriend will react negatively to your
 * instant message.
 */

typedef gdouble (*BayesCombiner) (BayesClassifier  *classifier,
                                  BayesGuess      **guesses,
                                  guint             len,
                                  const gchar      *name,
                                  gpointer          user_data);

struct _BayesClassifier
{
  GObject         parent_instance;
  BayesStorage   *storage;

  BayesTokenizer  token_func;
  gpointer        token_user_data;
  GDestroyNotify  token_notify;

  BayesCombiner   combiner_func;
  gpointer        combiner_user_data;
  GDestroyNotify  combiner_notify;
};

G_DEFINE_TYPE (BayesClassifier, bayes_classifier, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_STORAGE,
  LAST_PROP
};

static GParamSpec *properties [LAST_PROP];

static gdouble
bayes_classifier_robinson (BayesClassifier  *classifier,
                           BayesGuess      **guesses,
                           guint             len,
                           const gchar      *name,
                           gpointer          user_data)
{
  gdouble nth;
  gdouble P;
  gdouble Q;
  gdouble S;
  gdouble v;
  gdouble w;
  gdouble g;
  guint i;

  nth = 1.0 / (gdouble)len;

  v = 1.0;
  w = 1.0;

  for (i = 0; i < len; i++)
    {
      g = bayes_guess_get_probability (guesses[i]);
      v *= (1.0 - g);
      w *= g;
    }

  P = 1.0 - pow (v, nth);
  Q = 1.0 - pow (w, nth);
  S = (P - Q) / (P + Q);

  return (1 + S) / 2.0;
}

static gchar **
bayes_classifier_tokenize (BayesClassifier *self,
                           const gchar     *text)
{
  g_return_val_if_fail (BAYES_IS_CLASSIFIER (self), NULL);
  g_return_val_if_fail (text, NULL);

  return self->token_func (text, self->token_user_data);
}

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
BayesClassifier *
bayes_classifier_new (void)
{
  return g_object_new (BAYES_TYPE_CLASSIFIER, NULL);
}

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
void
bayes_classifier_train (BayesClassifier *self,
                        const gchar     *name,
                        const gchar     *text)
{
  gchar **tokens;
  guint i;

  g_return_if_fail (BAYES_IS_CLASSIFIER (self));
  g_return_if_fail (name);
  g_return_if_fail (text);

  if (NULL != (tokens = bayes_classifier_tokenize (self, text)))
    {
      for (i = 0; tokens[i]; i++)
        bayes_storage_add_token (self->storage, name, tokens [i]);
      g_strfreev (tokens);
    }
}

static gint
qsort_guesses (gconstpointer a,
               gconstpointer b)
{
  const BayesGuess *ag = *(const BayesGuess **)a;
  const BayesGuess *bg = *(const BayesGuess **)b;

  return (bg->probability - ag->probability) * 100.0;
}

static gint
sort_guesses (gconstpointer a,
              gconstpointer b)
{
  const BayesGuess *ag = (const BayesGuess *)a;
  const BayesGuess *bg = (const BayesGuess *)b;

  return (bg->probability - ag->probability) * 100.0;
}

static gdouble
bayes_classifier_combiner (BayesClassifier  *self,
                           BayesGuess      **guesses,
                           guint             len,
                           const gchar      *name)
{
  g_return_val_if_fail (BAYES_IS_CLASSIFIER (self), 0.0);
  g_return_val_if_fail (guesses, 0.0);
  g_return_val_if_fail (len, 0.0);
  g_return_val_if_fail (name, 0.0);

  return self->combiner_func (self, guesses, len, name, self->combiner_user_data);
}

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
 * [[
 * GList *list = bayes_classifier_guess (classifier, "who am i?");
 * g_list_foreach (list, (GFunc)bayes_guess_unref);
 * g_list_free (list);
 * ]]
 *
 * Returns: (transfer full) (element-type BayesGuess*): The guesses.
 */
GList *
bayes_classifier_guess (BayesClassifier *self,
                        const gchar     *text)
{
  BayesGuess *guess;
  GPtrArray *guesses;
  gdouble prob;
  gchar **tokens;
  gchar **names;
  GList *ret = NULL;
  guint i;
  guint j;

  g_return_val_if_fail (BAYES_IS_CLASSIFIER (self), NULL);
  g_return_val_if_fail (text, NULL);

  tokens = bayes_classifier_tokenize (self, text);
  names = bayes_storage_get_names (self->storage);

  for (i = 0; names[i]; i++)
    {
      guesses = g_ptr_array_new_with_free_func ((GDestroyNotify)bayes_guess_unref);

      for (j = 0; tokens[j]; j++)
        {
          prob = bayes_storage_get_token_probability (self->storage, names [i], tokens [j]);
          guess = bayes_guess_new (tokens[j], prob);
          g_ptr_array_add (guesses, guess);
        }

      g_ptr_array_sort (guesses, qsort_guesses);

      if (guesses->len != 0)
        {
          guess = bayes_guess_new (names[i],
                                   bayes_classifier_combiner (self,
                                                              (BayesGuess **)guesses->pdata,
                                                              guesses->len, names[i]));
          ret = g_list_prepend (ret, guess);
        }

      g_ptr_array_unref (guesses);
    }

  g_strfreev (names);
  g_strfreev (tokens);

  ret = g_list_sort (ret, sort_guesses);

  return ret;
}

/**
 * bayes_classifier_get_storage:
 * @self: (in): A #BayesClassifier.
 *
 * Retrieves the #BayesStorage used for tokens by @classifier.
 *
 * Returns: (transfer none): A #BayesStorage.
 */
BayesStorage *
bayes_classifier_get_storage (BayesClassifier *self)
{
  g_return_val_if_fail (BAYES_IS_CLASSIFIER (self), NULL);

  return self->storage;
}

/**
 * bayes_classifier_set_storage:
 * @self: (in): A #BayesClassifier.
 * @storage: (in) (allow-none): A #BayesStorage or %NULL.
 *
 * Sets the storage to use for tokens by the classifier.
 * If @storage is %NULL, then in memory storage will be used.
 */
void
bayes_classifier_set_storage (BayesClassifier *self,
                              BayesStorage    *storage)
{
  g_return_if_fail (BAYES_IS_CLASSIFIER (self));
  g_return_if_fail (!storage || BAYES_IS_STORAGE (storage));

  if (g_set_object (&self->storage, storage))
    g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_STORAGE]);
}

/**
 * bayes_classifier_set_tokenizer:
 * @self: (in): A #BayesClassifier.
 * @tokenizer: (in): A #BayesTokenizer.
 * @user_data: (in): User data for @tokenizer.
 * @notify: (in): Destruction notification for @user_data.
 *
 * Sets the tokenizer to use to tokenize input text by @classifer for
 * both training using bayes_classifier_train() and guessing using
 * bayes_classifier_guess().
 */
void
bayes_classifier_set_tokenizer (BayesClassifier *self,
                                BayesTokenizer   tokenizer,
                                gpointer         user_data,
                                GDestroyNotify   notify)
{
  g_return_if_fail (BAYES_IS_CLASSIFIER (self));
  g_return_if_fail (tokenizer || (!user_data && !notify));

  if (self->token_notify != NULL)
    self->token_notify (self->token_user_data);

  self->token_func = tokenizer ? tokenizer : bayes_tokenizer_word;
  self->token_user_data = tokenizer ? user_data : NULL;
  self->token_notify = tokenizer ? notify : NULL;
}

static void
bayes_classifier_set_combiner (BayesClassifier *self,
                               BayesCombiner    combiner,
                               gpointer         user_data,
                               GDestroyNotify   notify)
{
  g_return_if_fail (BAYES_IS_CLASSIFIER (self));
  g_return_if_fail (combiner || (!user_data && !notify));

  if (self->combiner_notify != NULL)
    self->combiner_notify (self->combiner_user_data);

  self->combiner_func = combiner ? combiner : bayes_classifier_robinson;
  self->combiner_user_data = combiner ? user_data : NULL;
  self->combiner_notify = combiner ? notify : NULL;
}

static void
bayes_classifier_finalize (GObject *object)
{
  BayesClassifier *self = (BayesClassifier *)object;

  bayes_classifier_set_tokenizer (self, NULL, NULL, NULL);
  bayes_classifier_set_combiner (self, NULL, NULL, NULL);
  g_clear_object (&self->storage);

  G_OBJECT_CLASS (bayes_classifier_parent_class)->finalize (object);
}

static void
bayes_classifier_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  BayesClassifier *self = BAYES_CLASSIFIER (object);

  switch (prop_id)
    {
    case PROP_STORAGE:
      g_value_set_object (value, bayes_classifier_get_storage (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
bayes_classifier_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  BayesClassifier *self = BAYES_CLASSIFIER (object);

  switch (prop_id)
    {
    case PROP_STORAGE:
      bayes_classifier_set_storage (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
bayes_classifier_class_init (BayesClassifierClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = bayes_classifier_finalize;
  object_class->get_property = bayes_classifier_get_property;
  object_class->set_property = bayes_classifier_set_property;

  /**
   * BayesClassifier:storage:
   *
   * The "storage" property. The training data used by @classifier is
   * saved to and loaded from @storage.
   */
  properties [PROP_STORAGE] =
    g_param_spec_object ("storage",
                         "Storage",
                         "The storage to use for tokens.",
                         BAYES_TYPE_STORAGE,
                         G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
bayes_classifier_init (BayesClassifier *self)
{
  bayes_classifier_set_tokenizer (self, NULL, NULL, NULL);
  bayes_classifier_set_combiner (self, NULL, NULL, NULL);
  bayes_classifier_set_storage (self, NULL);
}
