// valac {BayesStorageSerializable,trainer}.vala -g --enable-experimental --pkg Bayes-1.0 --pkg gio-2.0 --pkg json-glib-1.0 -o trainer

public class SourceCodeTrainer {
    static Bayes.Classifier classifier;

    // options
    private static string? samples_dir = null;
    private static string? output_file = null;
    private static string? tokenizer = "word";

    static bool train_file (string name, string filename) {
	    File file = File.new_for_path (filename);
	    uint8[] buf;
	    string etag;

	    try {
		    file.load_contents (null, out buf, out etag);
		    classifier.train (name, (string) buf);
		    return true;
	    } catch (Error e) {
		    stdout.printf ("error: %s\n", e.message);
		    return false;
	    }
    }
    
    static void train_test (string lang_dir, string language) {
        File samples = File.new_for_path (@"$samples_dir/$lang_dir");
        FileEnumerator enumerator = samples.enumerate_children ("standard::*", FileQueryInfoFlags.NOFOLLOW_SYMLINKS);

        FileInfo info = null;
        while ((info = enumerator.next_file ()) != null) {
            if (info.get_file_type () == FileType.DIRECTORY)
                continue;
            Test.message ("training with %s...", info.get_name ());
            if (!train_file (language, @"$samples_dir/$lang_dir/%s".printf(info.get_name ())))
		        Test.fail ();
        }
    }

    private const OptionEntry[] options = {
	    // --samples
	    { "samples", 'd', 0, OptionArg.FILENAME, ref samples_dir, "Directory where samples are located", "DIRECTORY" },
	    // --output
	    { "output", 'o', 0, OptionArg.FILENAME, ref output_file, "Output results to file", "FILE" },
	    // --tokenizer
	    { "tokenizer", 't', 0, OptionArg.STRING, ref tokenizer, "Tokenizer function", "'word' | 'code_tokens'" },

	    { null }
    };

    // generate file from tests
    static int main (string[] args) {
	try {
		var opt_context = new OptionContext ("- Generate Training Data");
		opt_context.add_main_entries (options, null);
		opt_context.set_help_enabled (true);
		opt_context.parse (ref args);
	} catch (OptionError e) {
		stdout.printf ("error: %s\n", e.message);
		stdout.printf ("Run '%s --help' to see available command-line options.\n", args[0]);
		return 0;
	}
	classifier = new Bayes.Classifier ();
	classifier.storage = new Bayes.StorageMemory ();

    // FIXME: bindings
	if (tokenizer == "word")
	    classifier.set_tokenizer (text => {
	        return Bayes.tokenizer_word (text, null);
	    });
	else if (tokenizer == "code_tokens")
	    classifier.set_tokenizer (text => {
	        return Bayes.tokenizer_code_tokens (text, null);
	    });

    Test.init (ref args);

	// training
	Test.add_func ("/Train/C", () => train_test ("C", "c"));
	Test.add_func ("/Train/C#", () => train_test ("C#", "c-sharp"));
	Test.add_func ("/Train/C++", () => train_test ("C++", "cpp"));
	Test.add_func ("/Train/Java", () => train_test ("Java", "java"));
	Test.add_func ("/Train/JavaScript", () => train_test ("JavaScript", "js"));
	Test.add_func ("/Train/JSON", () => train_test ("JSON", "json"));
	Test.add_func ("/Train/Haskell", () => train_test ("Haskell", "haskell-literate"));
	Test.add_func ("/Train/HTML", () => train_test ("HTML", "html"));
	Test.add_func ("/Train/Makefile", () => train_test ("Makefile", "makefile"));
	Test.add_func ("/Train/PHP", () => train_test ("PHP", "php"));
	Test.add_func ("/Train/Python", () => train_test ("Python", "python"));
	Test.add_func ("/Train/Scala", () => train_test ("Scala", "scala"));
	Test.add_func ("/Train/Shell", () => train_test ("Shell", "sh"));
	Test.add_func ("/Train/SQL", () => train_test ("SQL", "sql"));

	// serializing
	Test.add_func ("/Serialize/Training Data", () => {
		if ((classifier.storage as Bayes.StorageMemory).save_to_file (output_file))
			Test.message ("saved to %s", output_file);
		else
			Test.fail ();
	});

        return Test.run ();
    }
}
