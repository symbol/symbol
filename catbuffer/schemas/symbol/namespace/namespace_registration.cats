import "namespace/namespace_types.cats"
import "transaction.cats"

# Shared content between NamespaceRegistrationTransaction and EmbeddedNamespaceRegistrationTransaction.
inline struct NamespaceRegistrationTransactionBody
	# Number of confirmed blocks you would like to rent the namespace for. Required for root namespaces.
	duration = BlockDuration if ROOT equals registration_type

	# Parent namespace identifier. Required for sub-namespaces.
	parent_id = NamespaceId if CHILD equals registration_type

	# Namespace identifier.
	id = NamespaceId

	# Namespace registration type.
	registration_type = NamespaceRegistrationType

	# Namespace name size in bytes.
	name_size = uint8

	# Namespace name.
	name = array(uint8, name_size)

# Register (or renew a registration for) a [namespace](/concepts/namespace.html) (V1, latest).
#
# Namespaces help keep assets organized.
struct NamespaceRegistrationTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, NAMESPACE_REGISTRATION)

	inline Transaction
	inline NamespaceRegistrationTransactionBody

# Embedded version of NamespaceRegistrationTransaction (V1, latest).
struct EmbeddedNamespaceRegistrationTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, NAMESPACE_REGISTRATION)

	inline EmbeddedTransaction
	inline NamespaceRegistrationTransactionBody
