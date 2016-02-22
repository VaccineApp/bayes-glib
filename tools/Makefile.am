noinst_PROGRAMS = classifier trainer

classifier_SOURCES = classifier.vala
classifier_CFLAGS = \
	-Wno-incompatible-pointer-types \
	$(BAYES_GLIB_CFLAGS)
classifier_LDFLAGS = \
	$(BAYES_GLIB_LIBS) \
	-lbayes-glib-1.0 \
	-lm
classifier_VALAFLAGS = \
	--vapidir ../src \
	--pkg bayes-glib-1.0 \
	--pkg json-glib-1.0

trainer_SOURCES = trainer.vala
trainer_CFLAGS = \
	-Wno-incompatible-pointer-types \
	$(BAYES_GLIB_CFLAGS)
trainer_LDFLAGS = \
	$(BAYES_GLIB_LIBS) \
	-lbayes-glib-1.0 \
	-lm
trainer_VALAFLAGS = \
	--vapidir ../src \
	--pkg bayes-glib-1.0 \
	--pkg json-glib-1.0


CLEANFILES = *.h *.c *.o *.stamp \
	classifier classifier.c \
	trainer trainer.c
