bin_PROGRAMS = bayes-classifier bayes-trainer

bayes_classifier_SOURCES = classifier.vala

bayes_classifier_CFLAGS = \
	-Wno-incompatible-pointer-types \
	$(BAYES_GLIB_CFLAGS) \
	-I$(top_srcdir)/src

bayes_classifier_LDADD = \
	$(BAYES_GLIB_LIBS) \
	$(top_builddir)/src/libbayes-glib-1.0.la
	-lm

bayes_classifier_VALAFLAGS = \
	--vapidir $(top_builddir)/src \
	--pkg bayes-glib-1.0 \
	--pkg json-glib-1.0


bayes_trainer_SOURCES = trainer.vala

bayes_trainer_CFLAGS = \
	-Wno-incompatible-pointer-types \
	$(BAYES_GLIB_CFLAGS) \
	-I$(top_srcdir)/src

bayes_trainer_LDADD = \
	$(BAYES_GLIB_LIBS) \
	$(top_builddir)/src/libbayes-glib-1.0.la
	-lm

bayes_trainer_VALAFLAGS = \
	--vapidir $(top_builddir)/src \
	--pkg bayes-glib-1.0 \
	--pkg json-glib-1.0


CLEANFILES = *.h *.c *.stamp

-include $(top_srcdir)/git.mk
