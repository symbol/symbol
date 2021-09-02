# only comments immediately adjacent to a line will appear in the output.
# consecutive lines are concatenated with <SPACE>.
# a blank line inserts a new line `\n` in the output.

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
        comments = ''
        needs_separator = False
        for comment in self.comments:
            comment = comment.strip()

            if not comment:
                comments += '\n'
                needs_separator = False
            else:
                if needs_separator:
                    comments += ' '

                comments += comment
                needs_separator = True

        self.comments.clear()
        return {'comments': comments}
