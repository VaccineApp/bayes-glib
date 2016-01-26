TESTS =


test_cflags = \
	$(BAYES_GLIB_CFLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src

test_libs = \
	$(BAYES_GLIB_LIBS) \
	$(top_builddir)/src/libbayes-glib-1.0.la \
	-lm


TESTS += test-bayes-guess
test_bayes_guess_SOURCES = test-bayes-guess.c
test_bayes_guess_CFLAGS = $(test_cflags)
test_bayes_guess_LDADD = $(test_libs)


TESTS += test-bayes-storage-memory
test_bayes_storage_memory_SOURCES = test-bayes-storage-memory.c
test_bayes_storage_memory_CFLAGS = $(test_cflags)
test_bayes_storage_memory_LDADD = $(test_libs)


noinst_PROGRAMS = $(TESTS)


-include $(top_srcdir)/git.mk
