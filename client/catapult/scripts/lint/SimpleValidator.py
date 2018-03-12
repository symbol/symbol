class Line: # pylint: disable=too-few-public-methods
    def __init__(self, path, line, lineno, kind=''):
        self.path = path
        self.line = line
        self.lineno = lineno
        self.kind = kind

class SimpleValidator:
    def __init__(self, errorReporter):
        self.errorReporter = errorReporter

    # reset() is called per every file, before processing any lines
    # check() per every line in file
    # finalize() is called per every file, after processing all lines
    # formatError() for every error reported

    # it's ok to supress this warning, as reset() will be called for every file
    # so it's fine to set fields inside it
    # pylint: disable=attribute-defined-outside-init
    def reset(self, path):
        self.path = path

    def finalize(self):
        pass
