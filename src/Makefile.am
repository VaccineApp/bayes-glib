lib_LTLIBRARIES = libbayes-glib-1.0.la

pkgincludedir = $(includedir)/bayes-glib-1.0
pkginclude_HEADERS = \
	bayes-classifier.h \
	bayes-glib.h \
	bayes-guess.h \
	bayes-storage-memory.h \
	bayes-storage.h \
	bayes-tokenizer.h \
	bayes-version.h

libbayes_glib_1_0_la_SOURCES = \
	$(pkginclude_HEADERS) \
	bayes-classifier.c \
	bayes-guess.c \
	bayes-guess-private.h \
	bayes-storage-memory.c \
	bayes-storage.c \
	bayes-tokenizer.c

libbayes_glib_1_0_la_CFLAGS = $(BAYES_GLIB_CFLAGS)
libbayes_glib_1_0_la_LIBADD = $(BAYES_GLIB_LIBS) -lm

-include $(top_srcdir)/git.mk
