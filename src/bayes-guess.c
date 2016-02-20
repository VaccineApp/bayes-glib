/* bayes-guess.c
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

#include "bayes-guess.h"
#include "bayes-guess-private.h"

/**
 * SECTION:bayes-guess
 * @title: BayesGuess
 * @short_description: An immutable structure containing a probability.
 * @see_also: bayes_guess_get_name() bayes_guess_get_probability()
 *
 * #BayesGuess represents a guess at the classification of given input
 * data. The guess also includes the probability calculated for the input
 * being the classification. The guess structure contains the classification
 * name and a probability between 0.0 and 1.0.
 *
 * The #BayesGuess structure is a reference counted #GBoxed type. You can
 * reference the structure with bayes_guess_ref() and free the structure
 * with bayes_guess_unref().
 *
 */

G_DEFINE_BOXED_TYPE (BayesGuess, bayes_guess, bayes_guess_ref, bayes_guess_unref)

BayesGuess *
bayes_guess_new (const gchar *name,
                 gdouble      probability)
{
  BayesGuess *guess;

  g_return_val_if_fail (name, NULL);

  guess = g_slice_new0 (BayesGuess);
  guess->ref_count = 1;
  guess->name = g_strdup(name);
  guess->probability = CLAMP(probability, 0.0, 1.0);

  return guess;
}

/**
 * bayes_guess_ref:
 * @guess: (in): A #BayesGuess.
 *
 * Increments the reference count of @guess by one.
 *
 * Returns: The instance provided, @guess.
 */
BayesGuess *
bayes_guess_ref (BayesGuess *guess)
{
  g_return_val_if_fail (guess != NULL, NULL);
  g_return_val_if_fail (guess->ref_count > 0, NULL);

  g_atomic_int_inc (&guess->ref_count);

  return guess;
}

/**
 * bayes_guess_unref:
 * @guess: (in): A #BayesGuess.
 *
 * Decrements the reference count of @guess by one. Once the reference count
 * reaches zero, the structure and allocated resources are released.
 */
void
bayes_guess_unref (BayesGuess *guess)
{
  g_return_if_fail (guess != NULL);
  g_return_if_fail (guess->ref_count > 0);

  if (g_atomic_int_dec_and_test (&guess->ref_count))
    {
      g_free (guess->name);
      g_slice_free (BayesGuess, guess);
    }
}

const gchar *
bayes_guess_get_name (BayesGuess *guess)
{
  g_return_val_if_fail(guess, NULL);

  return guess->name;
}

gdouble
bayes_guess_get_probability (BayesGuess *guess)
{
  g_return_val_if_fail (guess, 0.0);

  return guess->probability;
}
