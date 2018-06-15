class CommentParser:
    """Reusable aspect parser for comment statements"""
    def __init__(self):
        self.comments = []

    def try_process_line(self, line):
        """Processes the current line if and only if it is a comment line"""
        if line.startswith('#'):
            self.comments.append(line[1:])
            return True

        return False

    def commit(self):
        """Postprocesses and clears all captured comments"""
        comments = ' '.join(comment.strip() for comment in self.comments)
        self.comments.clear()
        return {'comments': comments}
