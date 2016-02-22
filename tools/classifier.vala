// valac {BayesStorageSerializable,classifier}.vala -g --enable-experimental --pkg Bayes-1.0 --pkg gio-2.0 --pkg json-glib-1.0 -o classifier

public class SourceCodeClassifier {
    static Bayes.Classifier classifier;

    static GLib.List<Bayes.Guess> guess_file (string filename) {
	    File file = File.new_for_path (filename);
	    uint8[] buf;
	    string etag;

	    try {
		    file.load_contents (null, out buf, out etag);
		    return classifier.guess ((string) buf);
	    } catch (Error e) {
		    error (e.message);
	    }
    }

    static void guess_test (string filename) {
	    var guesses = guess_file (filename);
	    foreach (Bayes.Guess guess in guesses)
		stdout.printf (" %s (%f)\n", guess.get_name (), guess.get_probability ());
    }

    private static string? training_file = null;
    [CCode (array_length = false, array_null_terminated = true)]
    private static string[]? files = null;
    private static string? serialize_file = null;
    private static string? tokenizer = "word";

    private const OptionEntry[] options = {
	    // --training-file
	    { "training-file", 't', 0, OptionArg.FILENAME, ref training_file, "Training data file", "FILE"},
	    // --files
	    { "files", 'i', 0, OptionArg.FILENAME_ARRAY, ref files, "Files to classify", "FILE..." },
	    // --reserialize
	    { "reserialize", 'o', 0, OptionArg.FILENAME, ref serialize_file, "File to re-serialize training data to", "FILE"},
	    // --tokenizer
	    { "tokenizer", 0, 0, OptionArg.STRING, ref tokenizer, "Tokenizer function", "'word' | 'code_tokens'"},
	    { null }
    };

    // classify source code files
    static int main (string[] args) {
	try {
		var opt_context = new OptionContext ("- Classify Source Code");
		opt_context.add_main_entries (options, null);
		opt_context.set_help_enabled (true);
		opt_context.parse (ref args);
	} catch (OptionError e) {
		stdout.printf ("error: %s\n", e.message);
		stdout.printf ("Run '%s --help' to see available command-line options.\n", args[0]);
		return 0;
	}
	classifier = new Bayes.Classifier ();

    // FIXME: bindings
	if (tokenizer == "word")
	    classifier.set_tokenizer (text => {
	        return Bayes.tokenizer_word (text, null);
	    });
	else if (tokenizer == "code_tokens")
	    classifier.set_tokenizer (text => {
	        return Bayes.tokenizer_code_tokens (text, null);
	    });

	// deserializing
	try {
		classifier.storage = new Bayes.StorageMemory.from_file (training_file);
		stdout.printf ("loaded %s\n", training_file);
	} catch (Error e) {
		error (e.message);
	}

	if (serialize_file != null) {
		(classifier.storage as Bayes.StorageMemory).save_to_file (serialize_file);
		stdout.printf ("wrote to %s\n", serialize_file);
	}

	// guessing
	foreach (var file in files)
		if (file != null) {
			stdout.printf ("%s\n", file);
			guess_test (file);
		}

	return 0;
    }
}
