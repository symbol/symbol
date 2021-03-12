from abc import abstractmethod


class NisTransactionTestDescriptor:
    # pylint: disable=too-few-public-methods

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

    def test_deadline_is_updated_with_timestamp(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()
        transaction = test_descriptor.transaction_class(test_descriptor.network)

        # Act:
        transaction.timestamp = 9876

        # Assert:
        self.assertEqual(9876 + 60 * 60, transaction.deadline)

    @abstractmethod
    def get_test_descriptor(self):
        pass
