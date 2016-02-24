/* bayes-tokenizer.c
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

#include "bayes-tokenizer.h"

static GRegex *word_regex;

/**
 * SECTION:bayes-tokenizer
 * @title: BayesTokenizer
 * @short_description: Reusable tokenizers for input data.
 *
 * #BayesTokenizer callbacks are designed to allow customization on how
 * input text should be tokenized. For some tasks, this may be as simple
 * as splitting text on word boundries. Others may be more complicated.
 */

gchar **
bayes_tokenizer_word (const gchar *text,
                      gpointer     user_data)
{
  static gsize initialized = FALSE;
  GMatchInfo *match_info;
  GPtrArray *ret;
  gchar **strv;
  guint i;

  if (g_once_init_enter (&initialized))
    {
      word_regex = g_regex_new ("\\w+", G_REGEX_OPTIMIZE, 0, NULL);
      g_assert (word_regex);
      g_once_init_leave (&initialized, TRUE);
    }

  ret = g_ptr_array_new ();

  if (g_regex_match (word_regex, text, 0, &match_info))
    {
      while (g_match_info_matches (match_info))
        {
          strv = g_match_info_fetch_all (match_info);

          for (i = 0; strv[i]; i++)
            {
              g_ptr_array_add (ret, strv[i]);
              strv[i] = NULL;
            }

          g_free (strv);
          g_match_info_next (match_info, NULL);
        }
    }

  g_match_info_free (match_info);

  g_ptr_array_add (ret, NULL);

  return (gchar **)g_ptr_array_free (ret, FALSE);
}

gchar **
bayes_tokenizer_code_tokens (const gchar *text,
                             gpointer     user_data)
{
  struct Expr {
    const gchar *expr;
    gsize initialized;
    GRegex *regex;
  };
  static struct Expr expressions[] = {
      { "(?<![\\(\\<\"\\w\\$\\#\\%\\@])[A-Za-z_]\\w+(?![\\w\\>\"\\)])", FALSE, NULL },                        // symbols
      { "\\*+(?=[\\w\\[])", FALSE, NULL },             // pointers
      { "\\<\\w+\\>", FALSE, NULL },                  // type params
      { "(?<!:):(?!:)", FALSE, NULL },
      { "(?<=[\\w\\>])::(?=\\w)", FALSE, NULL },
      { " ::: ", FALSE, NULL },
      { " :: ", FALSE, NULL },
      { "(?<=[\\w\\)])\\.(?=\\w)", FALSE, NULL },
      { "(?<=[\\w\\)])\\.$", FALSE, NULL },
      { "(?<![:<>=])=(?![>=])", FALSE, NULL },
      { "[-=]{1,3}>", FALSE, NULL },
      { "(?<!<)={3,}(?!>)", FALSE, NULL },
      { ":=+(?![>=])", FALSE, NULL },
      { "\\$[\\(<\\^]", FALSE, NULL },
      { "\\|", FALSE, NULL },
      { "\\[\\]", FALSE, NULL },
      { "\\?", FALSE, NULL },
      { "\"\\w+\"", FALSE, NULL },
      { "\\$\\w+", FALSE, NULL },
      { "[{}]", FALSE, NULL },
      { "#\\w+(?![\\>\"])", FALSE, NULL },
      { "\\(\\w+\\)", FALSE, NULL },
      { "@\\w+", FALSE, NULL },
      { "%\\w+", FALSE, NULL },
      { "<\\w+\\.[a-z]+>", FALSE, NULL },
      { ";", FALSE, NULL },
      { "(?!</)//(?!/)", FALSE, NULL },
      { "///", FALSE, NULL },
      { "/\\*+", FALSE, NULL },
      { "\\*/(?!/)", FALSE, NULL },
      { "</\\w+>", FALSE, NULL },
      { 0, 0 }
  };
  GMatchInfo *match_info;
  GPtrArray *ret;
  gchar **strv;
  guint i;

  struct Expr *expr;

  ret = g_ptr_array_new ();

  for (expr = &expressions[0]; expr->expr; ++expr)
    {
      if (g_once_init_enter (&expr->initialized))
        {
          GError *error = NULL;
          expr->regex = g_regex_new (expr->expr, G_REGEX_OPTIMIZE, 0, &error);
          if (!expr->regex)
            {
              g_printerr ("failed to compile %s: %s", expr->expr, error->message);
              g_assert_not_reached ();
            }
          g_once_init_leave (&expr->initialized, TRUE);
        }

      if (g_regex_match (expr->regex, text, 0, &match_info))
        {
          while (g_match_info_matches (match_info))
            {
              strv = g_match_info_fetch_all (match_info);

              for (i = 0; strv[i]; i++)
                {
                  g_ptr_array_add (ret, strv[i]);
                  strv[i] = NULL;
                }

              g_free (strv);
              g_match_info_next (match_info, NULL);
            }
        }

      g_match_info_free (match_info);
    }

  g_ptr_array_add (ret, NULL);

  return (gchar **)g_ptr_array_free (ret, FALSE);
}
