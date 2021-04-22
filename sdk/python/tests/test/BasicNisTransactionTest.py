from abc import abstractmethod


class NisTransactionTestDescriptor:
    def __init__(self, transaction_class, transaction_name, transaction_type, network):
        self.transaction_class = transaction_class
        self.transaction_name = transaction_name
        self.transaction_type = transaction_type
        self.network = network


class BasicNisTransactionTest:
    # pylint: disable=no-member

    def test_constants_are_correct(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        # Act + Assert:
        self.assertEqual(test_descriptor.transaction_name, test_descriptor.transaction_class.NAME)
        self.assertEqual(test_descriptor.transaction_type, test_descriptor.transaction_class.TYPE)

    def test_timestamp_is_updated_with_deadline(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()
        transaction = test_descriptor.transaction_class(test_descriptor.network)

        # Act:
        transaction.deadline = 9876 + 24 * 60 * 60

        # Assert:
        self.assertEqual(9876 + 24 * 60 * 60, transaction.deadline)
        self.assertEqual(9876, transaction.timestamp)

    def test_timestamp_is_never_set_to_negative(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()
        transaction = test_descriptor.transaction_class(test_descriptor.network)

        # Act:
        transaction.deadline = 1

        # Assert:
        self.assertEqual(1, transaction.deadline)
        self.assertEqual(0, transaction.timestamp)

    @abstractmethod
    def get_test_descriptor(self):
        pass
