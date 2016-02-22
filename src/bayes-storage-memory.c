/* bayes-storage-memory.c
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

#include "bayes-storage-memory.h"
#include "bayes-storage-memory-private.h"
#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>

/**
 * SECTION:bayes-storage-memory
 * @title: BayesStorageMemory
 * @short_description: Storage of training data in memory.
 *
 * #BayesStorageMemory is an implementation of #BayesStorage that
 * stores the tokens and their associated counts in memory using
 * #GHashTable. It is mean for smaller data sets. It can be serialized
 * and deserialized from JSON format.
 */

static void bayes_storage_init (BayesStorageInterface *iface);

enum {
	PROP_0,

	PROP_NAMES,
	PROP_CORPUS,

	N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static JsonSerializableIface *serializable_iface = NULL;

static void json_serializable_iface_init (JsonSerializableIface *iface);

G_DEFINE_TYPE_EXTENDED (BayesStorageMemory,
                        bayes_storage_memory,
                        G_TYPE_OBJECT,
                        0,
                        G_IMPLEMENT_INTERFACE (BAYES_TYPE_STORAGE, bayes_storage_init);
			G_IMPLEMENT_INTERFACE (JSON_TYPE_SERIALIZABLE, json_serializable_iface_init))

static void
bayes_tokens_free (gpointer data)
{
  BayesTokens *tokens = data;

  if (tokens != NULL)
    {
      g_hash_table_unref (tokens->tokens);
      g_free (tokens);
    }
}

static gpointer
bayes_tokens_copy (gpointer data)
{
	BayesTokens *old_tokens = data;
	BayesTokens *tokens;

	if (old_tokens == NULL)
		return NULL;

	tokens = g_new0 (BayesTokens, 1);
	tokens->tokens = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

	GHashTableIter iter;
	gchar *key;
	guint *val;

	g_hash_table_iter_init (&iter, old_tokens->tokens);
	while (g_hash_table_iter_next (&iter, (gpointer *)&key, (gpointer *)&val))
		g_hash_table_insert (tokens->tokens, g_strdup (key), g_memdup (val, sizeof (*val)));

	tokens->count = old_tokens->count;
	
	return tokens;
}

static void
bayes_tokens_inc (BayesTokens *tokens,
        	  const gchar *token,
		  guint        count)
{
  guint *token_count;

  /*
   * Get the container for the token count or create it if necessary.
   */
  if (!(token_count = g_hash_table_lookup (tokens->tokens, token)))
    {
      token_count = g_new0 (guint, 1);
      g_hash_table_insert (tokens->tokens, g_strdup (token), token_count);
    }

  /*
   * Increment the count of the token.
   */
  *token_count += count;
  tokens->count += count;
}

/*
 * converts BayesTokens to { "tokens": {}, "count": (int) }
 */
static JsonNode *
bayes_tokens_serialize (gconstpointer boxed) {
	const BayesTokens *tokens = boxed;
	JsonObject *object;
	JsonNode *node;

	if (boxed == NULL)
		return json_node_new (JSON_NODE_NULL);

	object = json_object_new ();
	node = json_node_new (JSON_NODE_OBJECT);

	/*
	 * serialize HashTable<string,uint> member
	 */
	GHashTableIter iter;
	gchar *token; /* key */
	guint *count; /* value */
	JsonObject *table_object;

	table_object = json_object_new ();
	g_hash_table_iter_init (&iter, tokens->tokens);
	while (g_hash_table_iter_next (&iter, (gpointer *) &token, (gpointer *)&count))
		json_object_set_int_member (table_object, token, *count);

	json_object_set_object_member (object, "tokens", table_object);

	/*
	 * serialize token count
	 */
	json_object_set_int_member (object, "count", tokens->count);

	json_node_take_object (node, object);

	return node;
}

static void
bayes_tokens_hashtable_foreach (JsonObject *table_object,
				const gchar *name,
				JsonNode *node,
				gpointer _tokens)
{
	BayesTokens *tokens = _tokens;

	bayes_tokens_inc (tokens, name, (guint) json_node_get_int (node));
}

/*
 * { "tokens": {}, "count": (uint) }
 */
static gpointer
bayes_tokens_deserialize (JsonNode *node) {
	JsonObject *object;
	BayesTokens *tokens;

	if (json_node_get_node_type (node) != JSON_NODE_OBJECT)
		return NULL;

	object = json_node_get_object (node);

	tokens = g_new0(BayesTokens, 1);
	tokens->tokens = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

	/*
	 * deserialize HashTable<string,uint> member
	 */
	JsonObject *table_object;

	table_object = json_object_get_object_member (object, "tokens");
	json_object_foreach_member (table_object, bayes_tokens_hashtable_foreach, tokens);

	/*
	 * unnecessary to deserialize token count
	 */

	return tokens;
}

GType
bayes_tokens_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (type == 0)) {
		type = g_boxed_type_register_static ("BayesTokens", bayes_tokens_copy, bayes_tokens_free);

		/*
		 * register transform functions
		 */
		json_boxed_register_serialize_func (type, JSON_NODE_OBJECT, bayes_tokens_serialize);
		json_boxed_register_deserialize_func (type, JSON_NODE_OBJECT, bayes_tokens_deserialize);
	}

	return type;
}


BayesStorageMemory *
bayes_storage_memory_new (void)
{
  return g_object_new (BAYES_TYPE_STORAGE_MEMORY, NULL);
}

BayesStorageMemory *
bayes_storage_memory_new_from_file (const gchar *filename,
				    GError **error)
{
	JsonParser *parser = json_parser_new ();
	JsonNode *node;
	GObject *obj;
	
	if (!json_parser_load_from_file (parser, filename, error))
		return NULL;

	node = json_parser_get_root (parser);
	obj = json_gobject_deserialize (BAYES_TYPE_STORAGE_MEMORY, node);

	return BAYES_STORAGE_MEMORY (obj);
}

gboolean
bayes_storage_memory_save_to_file (BayesStorageMemory *self,
				   const gchar *filename,
				   GError **error)
{
	JsonGenerator *json_gen;
	JsonNode *root;
	gboolean ret;

	json_gen = json_generator_new ();
	root = json_gobject_serialize (G_OBJECT (self));
	json_generator_set_root (json_gen, root);

	json_node_free (root);

	ret = json_generator_to_file (json_gen, filename, error);

	g_object_unref (json_gen);

	return ret;
}


static void
bayes_storage_memory_add_token_count (BayesStorage *storage,
                                      const gchar  *name,
                                      const gchar  *token,
                                      guint         count)
{
  BayesStorageMemory *self = (BayesStorageMemory *)storage;
  BayesTokens *tokens;

  g_assert (BAYES_IS_STORAGE_MEMORY (self));
  g_assert (name);
  g_assert (token);

  /*
   * Get the classification hashtable or create it if necessary.
   */
  if (!(tokens = g_hash_table_lookup (self->names, name)))
    {
      tokens = g_new0 (BayesTokens, 1);
      tokens->tokens = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
      g_hash_table_insert (self->names, g_strdup (name), tokens);
    }

  bayes_tokens_inc (tokens, token, count);
  bayes_tokens_inc (self->corpus, token, count);
}

static guint
bayes_storage_memory_get_token_count (BayesStorage *storage,
                                      const gchar  *name,
                                      const gchar  *token)
{
  BayesStorageMemory *self = (BayesStorageMemory *)storage;
  BayesTokens *tokens;
  guint *token_count;

  g_assert (BAYES_IS_STORAGE_MEMORY (self));

  tokens = name ? g_hash_table_lookup(self->names, name) : self->corpus;

  if (tokens != NULL)
    {
      if (!token)
        return tokens->count;
      else if ((token_count = g_hash_table_lookup(tokens->tokens, token)))
        return *token_count;
    }

  return 0;
}

static gdouble
bayes_storage_memory_get_token_probability (BayesStorage *storage,
                                            const gchar  *name,
                                            const gchar  *token)
{
  BayesStorageMemory *self = (BayesStorageMemory *)storage;
  gdouble pool_count;
  gdouble them_count;
  gdouble tot_count;
  gdouble this_count;
  gdouble other_count;
  gdouble good_metric;
  gdouble bad_metric;
  gdouble f;
  BayesTokens *tokens;

  g_assert (BAYES_IS_STORAGE_MEMORY (self));
  g_assert (name);
  g_assert (token);

  if (!(tokens = g_hash_table_lookup(self->names, name)))
    return 0.0;

  pool_count = tokens->count;
  them_count = MAX (self->corpus->count - pool_count, 1);
  this_count = bayes_storage_memory_get_token_count (storage, name, token);
  tot_count = bayes_storage_memory_get_token_count (storage, NULL, token);
  other_count = tot_count - this_count;
  good_metric = (!pool_count) ? 1.0 : MIN (1.0, other_count / pool_count);
  bad_metric = MIN (1.0, this_count / them_count);
  f = bad_metric / (good_metric + bad_metric);

  if (ABS (f - 0.5) >= 0.1)
     return MAX (0.0001, MIN (0.9999, f));

  return 0.0;
}

static gchar **
bayes_storage_memory_get_names (BayesStorage *storage)
{
  BayesStorageMemory *self = (BayesStorageMemory *)storage;
  GHashTableIter iter;
  GPtrArray *ret;
  gchar *key;

  g_assert (BAYES_IS_STORAGE_MEMORY (self));

  ret = g_ptr_array_new ();
  g_hash_table_iter_init (&iter, self->names);
  while (g_hash_table_iter_next (&iter, (gpointer *)&key, NULL))
    g_ptr_array_add (ret, g_strdup (key));
  g_ptr_array_add (ret, NULL);

  return (gchar **)g_ptr_array_free(ret, FALSE);
}

static void
bayes_storage_hashtable_deserialize_foreach (JsonObject *table_object,
					     const gchar *member_name,
					     JsonNode *member_node,
					     gpointer data)
{
	GHashTable *table = data;
	BayesTokens *obj;
	
	obj = json_boxed_deserialize (BAYES_TYPE_TOKENS, member_node);
	g_assert (obj != NULL);

	g_hash_table_insert (table, g_strdup (member_name), obj);
}

static gboolean
bayes_storage_memory_deserialize_property (JsonSerializable *serializable,
					   const gchar *prop_name,
					   GValue *value,
					   GParamSpec *pspec,
					   JsonNode *prop_node)
{
	if (g_strcmp0 (prop_name, "names") == 0) {
		GHashTable *table;
		JsonObject *obj_table;

		table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, bayes_tokens_free);
		obj_table = json_node_get_object (prop_node);

		/*
		 * deserialize each key=>val pair
		 */
		json_object_foreach_member (obj_table, bayes_storage_hashtable_deserialize_foreach, table);

		// g_value_init (value, G_TYPE_HASH_TABLE);
                g_assert (G_VALUE_TYPE (value) == G_TYPE_HASH_TABLE);
		g_value_set_boxed (value, table);

		return TRUE;
	} else if (g_strcmp0 (prop_name, "corpus") == 0) {
		gpointer boxed;

		// g_value_init (value, BAYES_TYPE_TOKENS);
                g_assert (G_VALUE_TYPE (value) == BAYES_TYPE_TOKENS);
		boxed = json_boxed_deserialize (BAYES_TYPE_TOKENS, prop_node);
		g_assert (boxed != NULL);

		g_value_set_boxed (value, boxed);
		return TRUE;
	} else
		return serializable_iface->deserialize_property (serializable, prop_name,
								 value, pspec, prop_node);
}

static JsonNode *
bayes_storage_memory_serialize_property (JsonSerializable *serializable,
					 const gchar	*prop_name,
					 const GValue	*value,
					 GParamSpec	*pspec)
{
	JsonNode *node = NULL;

	// { "names": {}, "corpus": {} }
	if (g_strcmp0 (prop_name, "names") == 0) {
		GHashTable *boxed;	// HashTable<string,Tokens>
		JsonObject *table_obj;	// serialized hashtable
		JsonNode *val;		// BayesTokens struct

		table_obj = json_object_new ();
		node = json_node_new (JSON_NODE_OBJECT);

		boxed = g_value_get_boxed (value);

		// serialize all entries
		GHashTableIter iter;
		gchar *name;
		BayesTokens *tokens;

		g_hash_table_iter_init (&iter, boxed);
		while (g_hash_table_iter_next (&iter, (gpointer *)&name, (gpointer *)&tokens)) {
			val = json_boxed_serialize (BAYES_TYPE_TOKENS, tokens);
			json_object_set_member (table_obj, name, val);
		}

		json_node_take_object (node, table_obj);
	} else if (g_strcmp0 (prop_name, "corpus") == 0) {
		BayesTokens *boxed;

		boxed = g_value_get_boxed (value);
		node = json_boxed_serialize (BAYES_TYPE_TOKENS, boxed);
	} else
		node = serializable_iface->serialize_property (serializable, prop_name, value, pspec);

	return node;
}

static void
bayes_storage_memory_get_property (GObject    *object,
				   guint      prop_id,
				   GValue      *value,
				   GParamSpec  *pspec)
{
	BayesStorageMemory *self = (BayesStorageMemory *) object;

	switch (prop_id) {
	case PROP_NAMES:
		g_value_set_boxed (value, self->names);
		break;
	case PROP_CORPUS:
		g_value_set_boxed (value, self->corpus);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
bayes_storage_memory_set_property (GObject     *object,
				   guint       prop_id,
				   const GValue *value,
				   GParamSpec   *pspec)
{
	BayesStorageMemory *self = (BayesStorageMemory *) object;

	switch (prop_id) {
	case PROP_NAMES:
		self->names = g_value_dup_boxed (value);
		g_object_notify_by_pspec (object, obj_properties [PROP_NAMES]);
		break;
	case PROP_CORPUS:
		self->corpus = g_value_dup_boxed (value);
		g_object_notify_by_pspec (object, obj_properties [PROP_CORPUS]);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
bayes_storage_memory_finalize (GObject *object)
{
  BayesStorageMemory *self = (BayesStorageMemory *)object;

  g_hash_table_unref (self->names);
  bayes_tokens_free (self->corpus);

  G_OBJECT_CLASS (bayes_storage_memory_parent_class)->finalize (object);
}

static void
bayes_storage_memory_class_init (BayesStorageMemoryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = bayes_storage_memory_finalize;

  object_class->set_property = bayes_storage_memory_set_property;
  object_class->get_property = bayes_storage_memory_get_property;

  /**
   * BayesStorageMemory:names: (type GLib.HashTable(utf8,Bayes.Tokens))
   *
   * The list of class names and their tokens.
   */
  obj_properties[PROP_NAMES] =
	  g_param_spec_boxed ("names", "Names",
			      "A table of classes",
			      G_TYPE_HASH_TABLE,
			      G_PARAM_READWRITE | G_PARAM_PRIVATE |
			      G_PARAM_STATIC_STRINGS);
  /**
   * BayesStorageMemory:corpus:
   *
   * Corpus
   */
  obj_properties[PROP_CORPUS] =
	  g_param_spec_boxed ("corpus", "Corpus",
			      "All tokens among classes",
			      BAYES_TYPE_TOKENS,
			      G_PARAM_READWRITE | G_PARAM_PRIVATE |
			      G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties (object_class,
		  		     N_PROPERTIES, obj_properties);
}

static GParamSpec *
bayes_storage_memory_find_property (JsonSerializable *serializable,
                                    const gchar       *name)
{
  BayesStorageMemory *self;
  GObjectClass *klass;

  self = BAYES_STORAGE_MEMORY (serializable);
  g_assert (self != NULL);

  klass = G_OBJECT_GET_CLASS (self);
  return g_object_class_find_property (klass, name);
}

static void
bayes_storage_memory_init (BayesStorageMemory *self)
{
  BayesTokens *tokens;

  self->names = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, bayes_tokens_free);
  tokens = g_new0(BayesTokens, 1);
  tokens->tokens = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  self->corpus = tokens;
}

static void
bayes_storage_init (BayesStorageInterface *iface)
{
  iface->add_token_count = bayes_storage_memory_add_token_count;
  iface->get_names = bayes_storage_memory_get_names;
  iface->get_token_count = bayes_storage_memory_get_token_count;
  iface->get_token_probability = bayes_storage_memory_get_token_probability;
}

static void json_serializable_iface_init (JsonSerializableIface *iface) {
  serializable_iface = g_type_default_interface_peek (JSON_TYPE_SERIALIZABLE);

  iface->serialize_property = bayes_storage_memory_serialize_property;
  iface->deserialize_property = bayes_storage_memory_deserialize_property;
  iface->find_property = bayes_storage_memory_find_property;
}
