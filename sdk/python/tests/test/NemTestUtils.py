import random


class NemTestUtils:
    """NEM common test utilities."""

    # pylint: disable=too-few-public-methods

    @staticmethod
    def randbytes(count):
        """Generates random bytes. Polyfill for random.randbytes in python 3.9."""
        if hasattr(random, 'randbytes'):
            # pylint: disable=no-member
            return random.randbytes(count)

        return bytes([random.randint(0x00, 0xFF) for _ in range(0, count)])
