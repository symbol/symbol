class CatsParseException(Exception):
    """Exception raised when a parse error is encountered"""

    def __init__(self, args, ex=None):
        super().__init__(args)

        self.message = None
        self.scope = None

        if isinstance(args, str):
            self.message = args
        elif isinstance(args, list):
            self.scope = args

        if isinstance(ex, CatsParseException):
            self.message = ex.message
