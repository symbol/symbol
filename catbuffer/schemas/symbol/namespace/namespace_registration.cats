import "namespace/namespace_types.cats"
import "transaction.cats"

# binary layout for a namespace registration transaction
struct NamespaceRegistrationTransactionBody
	# namespace duration
	duration = BlockDuration if ROOT equals registration_type

	# parent namespace identifier
	parent_id = NamespaceId if CHILD equals registration_type

	# namespace identifier
	id = NamespaceId

	# namespace registration type
	registration_type = NamespaceRegistrationType

	# namespace name size
	name_size = uint8

	# namespace name
	name = array(uint8, name_size)

# binary layout for a non-embedded namespace registration transaction
struct NamespaceRegistrationTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, NAMESPACE_REGISTRATION)

	inline Transaction
	inline NamespaceRegistrationTransactionBody

# binary layout for an embedded namespace registration transaction
struct EmbeddedNamespaceRegistrationTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, NAMESPACE_REGISTRATION)

	inline EmbeddedTransaction
	inline NamespaceRegistrationTransactionBody
