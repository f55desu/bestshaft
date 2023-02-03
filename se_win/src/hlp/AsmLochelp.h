// In order to make changing the help file source files (the files that the help author uses to generate
// the help file) while maintaining the addin source code that uses the help file easier to do, the wizard
// suggests using this file and the definitions it contains. Changes to the names of the htm files or
// context identifiers by the help author should be reflected in the definitions below. Then simply
// rebuild the help file and the addin project.

// First define the context to be used when registering help context which is for use by Solid Edge AddIn Manager.
// Note that the default configuration for a wizard generated addin is such that this will bring up the intro.htm
// topic.
#define IntroContext                   1
// Now define command contexts.
#define ModalDialogBoxCommandContext   2
#define LocateCommandContext           3

// Command topics are associated with these contexts in the [ALIAS] section of the project file.
// Topics ca be used instead of context identifiers. Its up to the designer and the
// help author to decide which method is best. The two are basically equivalent as far as the end result is
// concerned.
