import unittest

from catbuffer_parser.CommentParser import CommentParser


class CommentParserTest(unittest.TestCase):
    def test_try_process_line_returns_true_only_for_comment_lines(self):
        # Arrange:
        parser = CommentParser()

        # Act + Assert:
        for line in ['# foo bar', '#', '# $$$']:
            self.assertTrue(parser.try_process_line(line))

        for line in [' # foo bar', 'foo bar']:
            self.assertFalse(parser.try_process_line(line))

    def test_no_comments_are_present_initially(self):
        # Arrange:
        parser = CommentParser()

        # Act:
        result = parser.commit()

        # Assert:
        self.assertEqual({'comments': ''}, result)

    def test_can_add_single_line_comment(self):
        # Arrange:
        parser = CommentParser()

        # Act:
        parser.try_process_line('# this is a comment')
        result = parser.commit()

        # Assert:
        self.assertEqual({'comments': 'this is a comment'}, result)

    def test_can_add_multi_line_comment(self):
        # Arrange:
        parser = CommentParser()

        # Act:
        parser.try_process_line('# this is a comment')
        parser.try_process_line('# foo bar')
        result = parser.commit()

        # Assert:
        self.assertEqual({'comments': 'this is a comment foo bar'}, result)

    def test_post_processing_removes_leading_and_trailing_whitespace_per_line(self):
        # Arrange:
        parser = CommentParser()

        # Act:
        parser.try_process_line('#    this is a comment   ')
        parser.try_process_line('#      foo   bar   ')
        result = parser.commit()

        # Assert:
        self.assertEqual({'comments': 'this is a comment foo   bar'}, result)

    def test_can_reuse_parser(self):
        # Arrange:
        parser = CommentParser()

        # Act:
        parser.try_process_line('# this is a comment')
        result1 = parser.commit()

        parser.try_process_line('# foo bar')
        result2 = parser.commit()

        # Assert:
        self.assertEqual({'comments': 'this is a comment'}, result1)
        self.assertEqual({'comments': 'foo bar'}, result2)
