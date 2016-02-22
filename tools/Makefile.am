bin_PROGRAMS = bayes-classifier bayes-trainer

bayes_classifier_SOURCES = classifier.vala
bayes_classifier_CFLAGS = \
	-Wno-incompatible-pointer-types \
	$(BAYES_GLIB_CFLAGS)
bayes_classifier_LDFLAGS = \
	$(BAYES_GLIB_LIBS) \
	-lbayes-glib-1.0 \
	-lm
bayes_classifier_VALAFLAGS = \
	--vapidir ../src \
	--pkg bayes-glib-1.0 \
	--pkg json-glib-1.0

bayes_trainer_SOURCES = trainer.vala
bayes_trainer_CFLAGS = \
	-Wno-incompatible-pointer-types \
	$(BAYES_GLIB_CFLAGS)
bayes_trainer_LDFLAGS = \
	$(BAYES_GLIB_LIBS) \
	-lbayes-glib-1.0 \
	-lm
bayes_trainer_VALAFLAGS = \
	--vapidir ../src \
	--pkg bayes-glib-1.0 \
	--pkg json-glib-1.0


CLEANFILES = *.h *.c *.o *.stamp \
	bayes-classifier classifier.c \
	bayes-trainer trainer.c
