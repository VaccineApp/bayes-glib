#include <bayes-glib.h>

static void
test1 (void)
{
   g_autoptr(BayesStorage) storage = NULL;

   storage = bayes_storage_memory_new ();
   bayes_storage_add_token (storage, "english", "turbo");
   bayes_storage_add_token (storage, "english", "brakes");
   bayes_storage_add_token (storage, "english", "suspension");
   g_assert_cmpint (1, ==, bayes_storage_get_token_count (storage, "english", "turbo"));
   g_assert_cmpint (1, ==, bayes_storage_get_token_count (storage, "english", "brakes"));
   g_assert_cmpint (1, ==, bayes_storage_get_token_count (storage, "english", "suspension"));
   g_assert_cmpint (0, ==, bayes_storage_get_token_count (storage, "english", "cops"));
}

gint
main (gint   argc,
      gchar *argv[])
{
   g_test_init (&argc, &argv, NULL);
   g_test_add_func ("/Storage/Memory/basic_tests", test1);
   return g_test_run ();
}
