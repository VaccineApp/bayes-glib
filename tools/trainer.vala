// valac {BayesStorageSerializable,trainer}.vala -g --enable-experimental --pkg Bayes-1.0 --pkg gio-2.0 --pkg json-glib-1.0 -o trainer

public class SourceCodeTrainer {
    static Bayes.Classifier classifier;

    // options
    private static string? samples_dir = null;
    private static string? output_file = null;

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
    
    static void train_test (string language) {
            File samples = File.new_for_path (@"$samples_dir/$language");
            FileEnumerator enumerator = samples.enumerate_children ("standard::*", FileQueryInfoFlags.NOFOLLOW_SYMLINKS);
            
            FileInfo info = null;
            while ((info = enumerator.next_file ()) != null) {
                if (info.get_file_type () == FileType.DIRECTORY)
                    continue;
                Test.message ("training with %s...", info.get_name ());
                if (!train_file (language, @"$samples_dir/$language/%s".printf(info.get_name ())))
			Test.fail ();
            }
    }

    private const OptionEntry[] options = {
	    // --samples
	    { "samples", 'd', 0, OptionArg.FILENAME, ref samples_dir, "Directory where samples are located", "DIRECTORY" },
	    // --output
	    { "output", 'o', 0, OptionArg.FILENAME, ref output_file, "Output results to file", "FILE" },

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

        Test.init (ref args);

	// training
	Test.add_func ("/Train/Assembly", () => train_test ("Assembly"));
	Test.add_func ("/Train/C", () => train_test ("C"));
	Test.add_func ("/Train/C#", () => train_test ("C#"));
	Test.add_func ("/Train/C++", () => train_test ("C++"));
	Test.add_func ("/Train/Java", () => train_test ("Java"));
	Test.add_func ("/Train/JavaScript", () => train_test ("JavaScript"));
	Test.add_func ("/Train/Haskell", () => train_test ("Haskell"));
	Test.add_func ("/Train/HTML", () => train_test ("HTML"));
	Test.add_func ("/Train/Python", () => train_test ("Python"));

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
