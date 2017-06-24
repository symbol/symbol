#pragma once
#include "catapult/model/Notifications.h"
#include "catapult/model/Transaction.h"
#include "catapult/model/TransactionPlugin.h"
#include "tests/test/core/BalanceTransfers.h"

namespace catapult { namespace mocks {

	// region mock notifications

/// Defines a mock notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_MOCK_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) \
	constexpr auto Mock_##DESCRIPTION##_Notification = model::MakeNotificationType( \
			(model::NotificationChannel::CHANNEL), \
			(static_cast<model::NotificationFacilityCode>(-1)), \
			CODE)

	/// A mock notification raised on the observer channel.
	DEFINE_MOCK_NOTIFICATION(Observer_1, 0xFFFF, Observer);

	/// A second mock notification raised on the observer channel.
	DEFINE_MOCK_NOTIFICATION(Observer_2, 0xFFFE, Observer);

	/// A mock notification raised on the validator channel.
	DEFINE_MOCK_NOTIFICATION(Validator_1, 0xFFFF, Validator);

	/// A second mock notification raised on the validator channel.
	DEFINE_MOCK_NOTIFICATION(Validator_2, 0xFFFE, Validator);

	/// A mock notification raised on all channels.
	DEFINE_MOCK_NOTIFICATION(All_1, 0xFFFF, All);

	/// A second mock notification raised on all channels.
	DEFINE_MOCK_NOTIFICATION(All_2, 0xFFFE, All);

	/// A hash notification raised on no channels.
	DEFINE_MOCK_NOTIFICATION(Hash, 0xFFFD, None);

#undef DEFINE_MOCK_NOTIFICATION

	/// Notifies the arrival of a hash.
	struct HashNotification : public model::Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Mock_Hash_Notification;

	public:
		/// Creates a hash notification around \a hash.
		explicit HashNotification(const Hash256& hash)
				: model::Notification(Notification_Type, sizeof(HashNotification))
				, Hash(hash)
		{}

	public:
		/// The hash.
		const Hash256& Hash;
	};

	// endregion

	// region mock transaction

#pragma pack(push, 1)

	/// Binary layout for a mock transaction body.
	template<typename THeader>
	struct MockTransactionBody : public THeader {
	private:
		using TransactionType = MockTransactionBody<THeader>;

	public:
		static constexpr model::EntityType Entity_Type = static_cast<model::EntityType>(0x4FFF);

		static constexpr uint8_t Current_Version = 0xFF;

	public:
		/// Binary layout for a variable data header.
		struct VariableDataHeader {
			/// The size of the data.
			uint16_t Size;
		};

	public:
		/// The transaction recipient.
		Key Recipient;

		/// The header for variable data.
		VariableDataHeader Data;

		// followed by data if Data.Size != 0

	private:
		template<typename T>
		static auto DataPtrT(T& transaction) {
			return transaction.Data.Size ? THeader::PayloadStart(transaction) : nullptr;
		}

	public:
		/// Returns a const pointer to the variable data contained in this transaction.
		const uint8_t* DataPtr() const {
			return DataPtrT(*this);
		}

		/// Returns a pointer to the variable data contained in this transaction.
		uint8_t* DataPtr() {
			return DataPtrT(*this);
		}

	public:
		// Calculates the real size of mock \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType& transaction) noexcept {
			return sizeof(TransactionType) + transaction.Data.Size;
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(Mock)

#pragma pack(pop)

	// endregion

	/// Creates a mock transaction with variable data composed of \a dataSize random bytes.
	std::unique_ptr<MockTransaction> CreateMockTransaction(uint16_t dataSize);

	/// Creates an embedded mock transaction with variable data composed of \a dataSize random bytes.
	std::unique_ptr<EmbeddedMockTransaction> CreateEmbeddedMockTransaction(uint16_t dataSize);

	/// Creates a mock transaction with a \a fee and \a transfers.
	std::unique_ptr<mocks::MockTransaction> CreateTransactionWithFeeAndTransfers(Amount fee, const test::BalanceTransfers& transfers);

	/// Mock transaction plugin options.
	enum class PluginOptionFlags {
		/// Default plugin options.
		Default,
		/// Configures the mock transaction plugin to not support embedding.
		Not_Embeddable,
		/// Configures the mock transaction plugin to publish extra transaction data as balance transfers.
		Publish_Transfers,
		/// Configures the mock transaction plugin to publish extra custom notifications.
		Publish_Custom_Notifications,
		/// Configures the mock transaction plugin to return a custom data buffer (equal to the mock transaction's payload sans header).
		Custom_Buffers
	};

	/// Returns \c true if \a options has \a flag set.
	bool IsPluginOptionFlagSet(PluginOptionFlags options, PluginOptionFlags flag);

	/// Creates a (mock) transaction plugin with the specified \a type.
	std::unique_ptr<model::TransactionPlugin> CreateMockTransactionPlugin(int type);

	/// Creates a (mock) transaction plugin with \a options.
	std::unique_ptr<model::TransactionPlugin> CreateMockTransactionPlugin(
			PluginOptionFlags options = PluginOptionFlags::Default);

	/// Creates a default transaction registry with a single registered (mock) transaction with \a options.
	std::unique_ptr<model::TransactionRegistry> CreateDefaultTransactionRegistry(
			PluginOptionFlags options = PluginOptionFlags::Default);
}}
