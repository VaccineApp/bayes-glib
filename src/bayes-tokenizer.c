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

#define _EXPR(expr, replace) { expr, FALSE, NULL, replace }

gchar **
bayes_tokenizer_code_tokens (const gchar *text,
                             gpointer     user_data)
{
  struct Expr {
    const gchar *expr;
    gsize initialized;
    GRegex *regex;
    const gchar *replace;
  };
  static struct Expr expressions[] = {
      _EXPR ("(?<![\\(\\<\"\\w\\$\\#\\%\\@])[A-Za-z_]\\w+(?![\\w\\>\"\\)])", ":word:\\0"),
      _EXPR ("\\*+(?=[\\w\\[])", "\\0"),
      _EXPR ("\\<\\w+\\>", ":angle1:"),
      _EXPR ("(?<!:):(?![:\\n])", "\\0"),
      _EXPR ("(?<!:):\\n", ":colon_newline:"),
      _EXPR ("(?<=[\\w\\>])::(?=\\w)", "\\0"),
      _EXPR (" ::: ", "\\0"),
      _EXPR (" :: ", "\\0"),
      _EXPR ("(?<=[\\w\\)])\\.(?=\\w)", "\\0"),
      _EXPR("(?<=[\\w\\)])\\.$", "\\0"),
      _EXPR("(?<![:<>=])=(?![>=])", "\\0"),
      _EXPR("[-=]{1,3}>", "\\0"),
      _EXPR("(?<!<)={3,}(?!>)", ":3+=:"),
      _EXPR(":=+(?![>=])", "::=:"),
      _EXPR("\\$[\\(<\\^]", "\\0"),
      _EXPR("\\|", "\\0"),
      _EXPR("\\[\\]", "\\0"),
      _EXPR("\\?", "\\0"),
      _EXPR("\"\\w+\"(?=:)", ":property:"),
      _EXPR("\\$\\w+", ":$word:"),
      _EXPR("{.*}", ":bracketed:"),
      _EXPR("#\\w+(?![\\>\"])", "\\0"),
      _EXPR("\\((\\w+)\\)\\s*(?=\\w)", ":cast:\\1"),
      _EXPR("\\((\\w+) ?\\*+\\)", ":castptr:\\1"),
      _EXPR("@\\w+", "\\0"),
      _EXPR("%\\w+", "\\0"),
      _EXPR("&\\w+", ":ref:"),
      _EXPR("<[\\w/]+\\.[a-z]+>", ":angle2:"),
      _EXPR(";\\n", ":end;:"),
      _EXPR(";(?!\\n)", ";"),
      _EXPR("(?!</)//(?!/)", "\\0"),
      _EXPR("///", "\\0"),
      _EXPR("/\\*", "\\0"),
      _EXPR("\\*/(?!/)", "\\0"),
      _EXPR("</\\w+>", ":end_tag:"),
      _EXPR("\\w+\\.\\w+\\(", ":object_call:"),
      _EXPR("\\w+->\\w+\\(", ":object_call_deref:"),
      { 0, 0 }
  };
  GMatchInfo *match_info;
  GPtrArray *ret;
  gchar **strv;
  gchar *strr;
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
                  strr = g_regex_replace (expr->regex, strv[i], -1, 0,
                                          expr->replace, 0, NULL);
                  g_ptr_array_add (ret, strr);
                  g_free (strv[i]);
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
