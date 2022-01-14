(alphabetic order)

checkProjectStructure.py 
 - main script that ties all elements together
 - contains lot of hax:
   * filtered namespaces (`*detail`, `*_types`)
   * special includes order treatment for external deps (`ref10`, `ripemd`, `sha3`, `boost`, `mongocxx`, `bsoncxx`)
   * others

colorPrint.py 
 - helper for debug color printing

cppLexer.py
 - cpp lexer used by forward validator and Parser (namespace parser)

DepsChecker.py
 - checks dependencies based on includes, according to rules in deps.config

exclusions.py 
 - validation exclusions

forwardsValidation.py 
 - right parser repors error in forward declarations only in header files
 - contains hack for handling empty newline after namespace

HeaderParser.py 
 - collects info about includes, various preprocessor related checks, order of includes is done directly inside checkProjectStructure (according to rules)

Parser.py
 - namespace parser, collects information about namespaces so that later, checkProjectStructure can check if file under given path has a proper namespace,
 - contains hack for `TEST_CLASS`
 - contains hack for `DEFINE_` macros

Rules.py :
 - contains rules used by plugins, extensions, tests and main code (in checkProjectStructure.py)

SimpleValidator.py 
 - interface for 'simple' validators

validation.py 
 - bunch of different simple (line-based) validators
