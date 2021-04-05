import unittest

from symbolchain.core.CodeWordsEncoder import CodeWords, CodeWordsEncoder


class CodeWordsTest(unittest.TestCase):
    def test_equality_is_supported(self):
        # Arrange:
        code_words = CodeWords(['cat', 'bird', 'dog'])

        # Act + Assert:
        self.assertEqual(code_words, CodeWords(['cat', 'bird', 'dog']))
        self.assertNotEqual(code_words, CodeWords(['cat', 'bird', 'DOG']))
        self.assertNotEqual(code_words, CodeWords(['cat', 'bird']))
        self.assertNotEqual(code_words, None)

    def test_string_is_supported(self):
        self.assertEqual('cat bird dog', str(CodeWords(['cat', 'bird', 'dog'])))

    def test_string_is_supported_custom_separator(self):
        # Arrange:
        code_words = CodeWords(['cat', 'bird', 'dog'])
        code_words.separator = '+'

        # Act + Assert:
        self.assertEqual('cat+bird+dog', str(code_words))


WORD_LIST = ['alpha', 'beta', 'gamma', 'delta', 'epsilon', 'eta', 'theta', 'iota', 'kappa', 'lambda']


class CodeWordsEncoderTest(unittest.TestCase):
    # region wordlists

    def test_cannot_create_encoder_with_less_than_two_words(self):
        # Act + Assert:
        for wordlist in [[], ['O']]:
            with self.assertRaises(ValueError):
                CodeWordsEncoder(wordlist)

    def test_can_create_encoder_with_exactly_two_words(self):
        # Arrange:
        encoder = CodeWordsEncoder(['O', 'X'])

        # Act:
        code_words = encoder.encode_int(10)
        value = encoder.decode_int(code_words)

        # Assert:
        self.assertEqual(CodeWords(['O', 'X', 'O', 'X']), code_words)
        self.assertEqual(10, value)

    def _assert_can_create_encoder_with_bip_wordlist(self, encoder, expected_words):
        # Arrange:
        value_to_encode = 3 + 1712 * 2048 + 350 * 2048 * 2048

        # Act:
        code_words = encoder.encode_int(value_to_encode)
        value = encoder.decode_int(code_words)

        # Assert:
        self.assertEqual(CodeWords(expected_words), code_words)
        self.assertEqual(value_to_encode, value)

    def test_can_create_encoder_with_default_bip_wordlist(self):
        self._assert_can_create_encoder_with_bip_wordlist(CodeWordsEncoder(), ['about', 'stock', 'cloth'])

    def test_can_create_encoder_with_custom_bip_wordlist(self):
        self._assert_can_create_encoder_with_bip_wordlist(CodeWordsEncoder('spanish'), ['abierto', 'ser', 'carro'])

    # endregion

    # region encode_int

    def _assert_can_encode_int(self, value, expected_words):
        # Arrange:
        encoder = CodeWordsEncoder(WORD_LIST)

        # Act:
        code_words = encoder.encode_int(value)

        # Assert:
        self.assertEqual(CodeWords(expected_words), code_words)

    def test_can_encode_int_zero(self):
        self._assert_can_encode_int(0, ['alpha'])

    def test_can_encode_int_to_single_word(self):
        self._assert_can_encode_int(7, ['iota'])

    def test_can_encode_int_to_multiple_words(self):
        self._assert_can_encode_int(625, ['eta', 'gamma', 'theta'])

    # endregion

    # region decode_int

    def _assert_can_decode_int(self, words, expected_value):
        # Arrange:
        encoder = CodeWordsEncoder(WORD_LIST)

        # Act:
        value = encoder.decode_int(CodeWords(words))

        # Assert:
        self.assertEqual(expected_value, value)

    def test_can_decode_int_zero(self):
        self._assert_can_decode_int(['alpha'], 0)

    def test_can_decode_int_to_single_word(self):
        self._assert_can_decode_int(['iota'], 7)

    def test_can_decode_int_to_multiple_words(self):
        self._assert_can_decode_int(['eta', 'gamma', 'theta'], 625)

    # endregion
