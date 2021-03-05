import hashlib
import math

from mnemonic import Mnemonic


class DiceMnemonicGenerator:
    """Generates BIP39 mnemonics from dice rolls."""

    def __init__(self, num_die_sides=6, mnemonic_language='english'):
        """Creates a generator."""
        self.num_die_sides = num_die_sides
        self.mnemonic_language = mnemonic_language
        self.rolls = []

    def add_roll(self, value):
        """Adds a roll."""
        if 1 <= value <= self.num_die_sides:
            self.rolls.append(value)
            return

        raise ValueError('roll value must be between 1 and {}, inclusive'.format(self.num_die_sides))

    def frequencies(self):
        """Gets the number of times each number was rolled."""
        counts = [0] * self.num_die_sides
        for roll in self.rolls:
            counts[roll - 1] += 1

        return counts

    def to_mnemonic(self, shrink_wrap=False):
        """Generates a mnemonic by hashing the roll input. Disabling `shrink_wrap` is the same method used by coldcard wallet."""
        seed = hashlib.sha256(''.join(map(str, self.rolls)).encode('utf8')).digest()
        if not shrink_wrap:
            return (self._seed_to_mnemonic(seed), min(256, len(self.rolls) * -math.log2(1/self.num_die_sides)))

        seed_length = self._calculate_shrink_wrap_seed_length()
        return (self._seed_to_mnemonic(seed[0:seed_length]), 8 * seed_length)

    def _calculate_shrink_wrap_seed_length(self):
        min_seed_length = 16
        max_seed_length = 32

        total_roll_possibilities = self.num_die_sides ** len(self.rolls)
        if total_roll_possibilities < 2 ** (8 * min_seed_length):
            raise ValueError('shrink_wrap requires at least 128 bits of entropy but only {} bits provided'.format(total_roll_possibilities))

        seed_length = min_seed_length
        while 2 ** (8 * (seed_length + 4)) < total_roll_possibilities:
            seed_length += 4

        return min(max_seed_length, seed_length)

    def _seed_to_mnemonic(self, seed):
        return Mnemonic(self.mnemonic_language).to_mnemonic(seed)
