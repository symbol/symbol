from mnemonic import Mnemonic


class CodeWords:
    """Represents a group of words that encode some binary data."""

    def __init__(self, words):
        self.words = words
        self.separator = ' '

    def __eq__(self, other):
        return isinstance(other, CodeWords) and self.words == other.words

    def __str__(self):
        return self.separator.join(self.words)


class CodeWordsEncoder:
    """Encodes and decodes values using wordlists."""

    def __init__(self, wordlist='english'):
        self.wordlist = wordlist if not isinstance(wordlist, str) else Mnemonic(wordlist).wordlist
        self.wordlist_size = len(self.wordlist)

        if self.wordlist_size < 2:
            raise ValueError('wordlist must contain at least two words')

    def encode_int(self, value):
        """Encodes an integer into one or more code words."""
        seed = []
        while value:
            index = value % self.wordlist_size
            seed.append(self.wordlist[index])
            value = value // self.wordlist_size

        return CodeWords(seed if seed else [self.wordlist[0]])

    def decode_int(self, code_words):
        """Decodes one or more code words into an integer."""
        base = 1
        value = 0
        for word in code_words.words:
            index = self.wordlist.index(word)
            value += index * base
            base *= self.wordlist_size

        return value
