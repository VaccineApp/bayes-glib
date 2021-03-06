/* bayes-guess.h
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

#ifndef BAYES_GUESS_H
#define BAYES_GUESS_H

#include <glib-object.h>

G_BEGIN_DECLS

#define BAYES_TYPE_GUESS (bayes_guess_get_type())

typedef struct _BayesGuess BayesGuess;

/**
 * bayes_guess_get_name:
 * @guess: (in): A #BayesGuess.
 *
 * Retrieves the classification name for which this guess represents.
 *
 * Returns: A string which should not be modified or freed.
 */
const gchar *bayes_guess_get_name        (BayesGuess *guess);

/**
 * bayes_guess_get_probability:
 * @guess: (in): A #BayesGuess.
 *
 * Retrieves the probability that the input data matches the classification
 * this guess represents.
 *
 * Returns: A #gdouble between 0.0 and 1.0.
 */
gdouble      bayes_guess_get_probability (BayesGuess *guess);
GType        bayes_guess_get_type        (void);

/**
 * bayes_guess_new:
 * @name: (in): The name of the classification.
 * @probability: (in): The probability of the classification.
 *
 * Creates a new #BayesGuess that is the probability of a given
 * classification.
 *
 * Returns: (transfer full): A newly allocated #BayesGuess.
 */
BayesGuess  *bayes_guess_new             (const gchar *name,
                                          gdouble      probability);
BayesGuess  *bayes_guess_ref             (BayesGuess *guess);
void         bayes_guess_unref           (BayesGuess *guess);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (BayesGuess, bayes_guess_unref)

G_END_DECLS

#endif /* BAYES_GUESS_H */

