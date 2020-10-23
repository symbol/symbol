/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "ContainerTypes.h"
#include "EntityType.h"
#include "LinkAction.h"
#include "NetworkIdentifier.h"
#include "NotificationType.h"
#include "Resolvable.h"
#include "catapult/utils/ArraySet.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/plugins.h"
#include "catapult/types.h"
#include <vector>

namespace catapult { namespace model {

	// region base notification

	/// Basic notification.
	struct PLUGIN_API_DEPENDENCY Notification {
	public:
		/// Creates a new notification with \a type and \a size.
		Notification(NotificationType type, size_t size)
				: Type(type)
				, Size(size)
		{}

	public:
		/// Notification type.
		NotificationType Type;

		/// Notification size.
		size_t Size;
	};

	// endregion

	// region account

	/// Notification of use of an account address.
	struct AccountAddressNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Register_Account_Address_Notification;

	public:
		/// Creates a notification around \a address.
		explicit AccountAddressNotification(const ResolvableAddress& address)
				: Notification(Notification_Type, sizeof(AccountAddressNotification))
				, Address(address)
		{}

	public:
		/// Address (resolvable).
		ResolvableAddress Address;
	};

	/// Notification of use of an account public key.
	struct AccountPublicKeyNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Register_Account_Public_Key_Notification;

	public:
		/// Creates a notification around \a publicKey.
		explicit AccountPublicKeyNotification(const Key& publicKey)
				: Notification(Notification_Type, sizeof(AccountPublicKeyNotification))
				, PublicKey(publicKey)
		{}

	public:
		/// Public key.
		const Key& PublicKey;
	};

	// endregion

	// region balance

	/// Basic balance notification.
	template<typename TDerivedNotification>
	struct BasicBalanceNotification : public Notification {
	public:
		/// Creates a notification around \a sender, \a mosaicId and \a amount.
		BasicBalanceNotification(const Address& sender, UnresolvedMosaicId mosaicId, Amount amount)
				: Notification(TDerivedNotification::Notification_Type, sizeof(TDerivedNotification))
				, Sender(sender)
				, MosaicId(mosaicId)
				, Amount(amount)
		{}

	public:
		/// Sender.
		Address Sender;

		/// Mosaic id.
		UnresolvedMosaicId MosaicId;

		/// Amount.
		catapult::Amount Amount;
	};

	/// Notifies a balance transfer from sender to recipient.
	struct BalanceTransferNotification : public BasicBalanceNotification<BalanceTransferNotification> {
	public:
		/// Balance transfer amount types.
		enum class AmountType { Static, Dynamic };

	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Balance_Transfer_Notification;

	public:
		/// Creates a notification around \a sender, \a recipient, \a mosaicId and \a amount
		/// with optional amount type (\a transferAmountType) indicating interpretation of transfer amount.
		BalanceTransferNotification(
				const Address& sender,
				const UnresolvedAddress& recipient,
				UnresolvedMosaicId mosaicId,
				catapult::Amount amount,
				AmountType transferAmountType = AmountType::Static)
				: BasicBalanceNotification(sender, mosaicId, amount)
				, Recipient(recipient)
				, TransferAmountType(transferAmountType)
		{}

	public:
		/// Recipient.
		UnresolvedAddress Recipient;

		/// Amount type indicating interpretation of transfer amount.
		AmountType TransferAmountType;
	};

	/// Notifies a balance debit by sender.
	struct BalanceDebitNotification : public BasicBalanceNotification<BalanceDebitNotification> {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Balance_Debit_Notification;

	public:
		using BasicBalanceNotification<BalanceDebitNotification>::BasicBalanceNotification;
	};

	// endregion

	// region entity

	/// Notifies the arrival of an entity.
	struct EntityNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Entity_Notification;

	public:
		/// Creates an entity notification around \a networkIdentifier, \a entityType, \a entityVersion, \a minVersion and \a maxVersion.
		EntityNotification(
				model::NetworkIdentifier networkIdentifier,
				model::EntityType entityType,
				uint8_t entityVersion,
				uint8_t minVersion,
				uint8_t maxVersion)
				: Notification(Notification_Type, sizeof(EntityNotification))
				, NetworkIdentifier(networkIdentifier)
				, EntityType(entityType)
				, EntityVersion(entityVersion)
				, MinVersion(minVersion)
				, MaxVersion(maxVersion)
		{}

	public:
		/// Network identifier.
		model::NetworkIdentifier NetworkIdentifier;

		/// Entity type.
		model::EntityType EntityType;

		/// Entity version.
		uint8_t EntityVersion;

		/// Minimum supported version.
		uint8_t MinVersion;

		/// Maximum supported version.
		uint8_t MaxVersion;
	};

	// endregion

	// region block

	/// Notifies the arrival of a block.
	struct BlockNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Block_Notification;

	public:
		/// Creates a block notification around \a harvester, \a beneficiary, \a timestamp, \a difficulty and \a feeMultiplier.
		BlockNotification(
				const Address& harvester,
				const Address& beneficiary,
				Timestamp timestamp,
				Difficulty difficulty,
				BlockFeeMultiplier feeMultiplier)
				: Notification(Notification_Type, sizeof(BlockNotification))
				, Harvester(harvester)
				, Beneficiary(beneficiary)
				, Timestamp(timestamp)
				, Difficulty(difficulty)
				, FeeMultiplier(feeMultiplier)
				, NumTransactions(0)
		{}

	public:
		/// Block harvester.
		Address Harvester;

		/// Block beneficiary.
		Address Beneficiary;

		/// Block timestamp.
		catapult::Timestamp Timestamp;

		/// Block difficulty.
		catapult::Difficulty Difficulty;

		/// Block fee multiplier.
		BlockFeeMultiplier FeeMultiplier;

		/// Total block fee.
		Amount TotalFee;

		/// Number of block transactions.
		uint32_t NumTransactions;
	};

	/// Notifies the arrival of a block (type information only).
	struct BlockTypeNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Block_Type_Notification;

	public:
		/// Creates a block type notification around \a blockType and \a blockHeight.
		BlockTypeNotification(EntityType blockType, Height blockHeight)
				: Notification(Notification_Type, sizeof(BlockTypeNotification))
				, BlockType(blockType)
				, BlockHeight(blockHeight)
		{}

	public:
		/// Block type.
		EntityType BlockType;

		/// Block height.
		Height BlockHeight;
	};

	/// Notifies the arrival of an importance block.
	struct ImportanceBlockNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Block_Importance_Notification;

	public:
		/// Creates an importance block notification around \a votingEligibleAccountsCount, \a harvestingEligibleAccountsCount,
		/// \a totalVotingBalance and \a previousImportanceBlockHash.
		ImportanceBlockNotification(
				uint32_t votingEligibleAccountsCount,
				uint64_t harvestingEligibleAccountsCount,
				Amount totalVotingBalance,
				const Hash256& previousImportanceBlockHash)
				: Notification(Notification_Type, sizeof(ImportanceBlockNotification))
				, VotingEligibleAccountsCount(votingEligibleAccountsCount)
				, HarvestingEligibleAccountsCount(harvestingEligibleAccountsCount)
				, TotalVotingBalance(totalVotingBalance)
				, PreviousImportanceBlockHash(previousImportanceBlockHash)
		{}

	public:
		/// Number of voting eligible accounts.
		uint32_t VotingEligibleAccountsCount;

		/// Number of harvesting eligible accounts.
		uint64_t HarvestingEligibleAccountsCount;

		/// Total balance eligible for voting.
		Amount TotalVotingBalance;

		/// Previous importance block hash.
		const Hash256& PreviousImportanceBlockHash;
	};

	// endregion

	// region transaction

	/// Notifies the arrival of a transaction.
	struct TransactionNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Transaction_Notification;

	public:
		/// Creates a transaction notification around \a sender, \a transactionHash, \a transactionType and \a deadline.
		TransactionNotification(const Address& sender, const Hash256& transactionHash, EntityType transactionType, Timestamp deadline)
				: Notification(Notification_Type, sizeof(TransactionNotification))
				, Sender(sender)
				, TransactionHash(transactionHash)
				, TransactionType(transactionType)
				, Deadline(deadline)
		{}

	public:
		/// Transaction sender.
		Address Sender;

		/// Transaction hash.
		const Hash256& TransactionHash;

		/// Transaction type.
		EntityType TransactionType;

		/// Transaction deadline.
		Timestamp Deadline;
	};

	/// Notifies the arrival of a transaction deadline.
	struct TransactionDeadlineNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Transaction_Deadline_Notification;

	public:
		/// Creates a transaction deadline notification around \a deadline and \a maxLifetime.
		TransactionDeadlineNotification(Timestamp deadline, utils::TimeSpan maxLifetime)
				: Notification(Notification_Type, sizeof(TransactionDeadlineNotification))
				, Deadline(deadline)
				, MaxLifetime(maxLifetime)
		{}

	public:
		/// Transaction deadline.
		Timestamp Deadline;

		/// Custom maximum transaction lifetime.
		/// \note If \c 0, default network-specific maximum will be used.
		utils::TimeSpan MaxLifetime;
	};

	/// Notifies the arrival of a transaction fee.
	struct TransactionFeeNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Transaction_Fee_Notification;

	public:
		/// Creates a transaction fee notification around \a sender, \a transactionSize, \a fee and \a maxFee.
		TransactionFeeNotification(const Address& sender, uint32_t transactionSize, Amount fee, Amount maxFee)
				: Notification(Notification_Type, sizeof(TransactionFeeNotification))
				, Sender(sender)
				, TransactionSize(transactionSize)
				, Fee(fee)
				, MaxFee(maxFee)
		{}

	public:
		/// Transaction sender.
		Address Sender;

		/// Transaction size.
		uint32_t TransactionSize;

		/// Transaction fee.
		Amount Fee;

		/// Maximum transaction fee.
		Amount MaxFee;
	};

	// endregion

	// region signature

	/// Notifies the presence of a signature.
	struct SignatureNotification : public Notification {
	public:
		/// Replay protection modes.
		enum class ReplayProtectionMode { Enabled, Disabled };

	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Signature_Notification;

	public:
		/// Creates a signature notification around \a signerPublicKey, \a signature and \a data with optional replay protection mode
		/// (\a dataReplayProtectionMode) applied to data.
		SignatureNotification(
				const Key& signerPublicKey,
				const Signature& signature,
				const RawBuffer& data,
				ReplayProtectionMode dataReplayProtectionMode = ReplayProtectionMode::Disabled)
				: Notification(Notification_Type, sizeof(SignatureNotification))
				, SignerPublicKey(signerPublicKey)
				, Signature(signature)
				, Data(data)
				, DataReplayProtectionMode(dataReplayProtectionMode)
		{}

	public:
		/// Signer public key.
		const Key& SignerPublicKey;

		/// Signature.
		const catapult::Signature& Signature;

		/// Signed data.
		RawBuffer Data;

		/// Replay protection mode applied to data.
		ReplayProtectionMode DataReplayProtectionMode;
	};

	// endregion

	// region address interaction

	/// Notifies that a source address interacts with participant addresses.
	/// \note This notification cannot be used by an observer.
	struct AddressInteractionNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Address_Interaction_Notification;

	public:
		/// Creates a notification around \a source, \a transactionType and \a participantsByAddress.
		AddressInteractionNotification(
				const Address& source,
				EntityType transactionType,
				const UnresolvedAddressSet& participantsByAddress)
				: Notification(Notification_Type, sizeof(AddressInteractionNotification))
				, Source(source)
				, TransactionType(transactionType)
				, ParticipantsByAddress(participantsByAddress)
		{}

	public:
		/// Source.
		Address Source;

		/// Transaction type.
		EntityType TransactionType;

		/// Participants given by address.
		UnresolvedAddressSet ParticipantsByAddress;
	};

	// endregion

	// region mosaic required

	/// Notification of a required mosaic.
	struct MosaicRequiredNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Mosaic_Required_Notification;

	public:
		/// Creates a notification around \a owner, \a mosaicId and optional \a propertyFlagMask.
		MosaicRequiredNotification(const ResolvableAddress& owner, const ResolvableMosaicId& mosaicId, uint8_t propertyFlagMask = 0)
				: Notification(Notification_Type, sizeof(MosaicRequiredNotification))
				, Owner(owner)
				, MosaicId(mosaicId)
				, PropertyFlagMask(propertyFlagMask)
		{}

	public:
		/// Mosaic owner (resolvable).
		ResolvableAddress Owner;

		/// Mosaic id (resolvable).
		ResolvableMosaicId MosaicId;

		/// Mask of required property flags that must be set on the mosaic.
		uint8_t PropertyFlagMask;
	};

	// endregion

	// region source change

	/// Notification of a source change.
	struct SourceChangeNotification : public Notification {
	public:
		/// Source change types.
		enum class SourceChangeType { Absolute, Relative };

	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Source_Change_Notification;

	public:
		/// Creates a notification around \a primaryChangeType, \a primaryId, \a secondaryChangeType and \a secondaryId.
		SourceChangeNotification(
				SourceChangeType primaryChangeType,
				uint32_t primaryId,
				SourceChangeType secondaryChangeType,
				uint32_t secondaryId)
				: Notification(Notification_Type, sizeof(SourceChangeNotification))
				, PrimaryChangeType(primaryChangeType)
				, PrimaryId(primaryId)
				, SecondaryChangeType(secondaryChangeType)
				, SecondaryId(secondaryId)
		{}

	public:
		/// Type of primary source change.
		SourceChangeType PrimaryChangeType;

		/// Primary source (e.g. index within block).
		uint32_t PrimaryId;

		/// Type of secondary source change.
		SourceChangeType SecondaryChangeType;

		/// Secondary source (e.g. index within aggregate).
		uint32_t SecondaryId;
	};

	// endregion

	// region padding

	/// Notification of internal padding.
	struct InternalPaddingNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Internal_Padding_Notification;

	public:
		/// Creates a notification around \a padding.
		explicit InternalPaddingNotification(uint64_t padding)
				: Notification(Notification_Type, sizeof(InternalPaddingNotification))
				, Padding(padding)
		{}

	public:
		/// Padding data.
		uint64_t Padding;
	};

	// endregion

	// region key link

	/// Notification of a key link action.
	struct KeyLinkActionNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Core_Key_Link_Action_Notification;

	public:
		/// Creates a notification around \a linkAction.
		explicit KeyLinkActionNotification(model::LinkAction linkAction)
				: Notification(Notification_Type, sizeof(KeyLinkActionNotification))
				, LinkAction(linkAction)
		{}

	public:
		/// Link action.
		model::LinkAction LinkAction;
	};

	/// Notification of a key link.
	template<typename TAccountPublicKey, NotificationType Key_Link_Notification_Type>
	struct BasicKeyLinkNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Key_Link_Notification_Type;

	public:
		/// Creates a notification around \a mainAccountPublicKey, \a linkedPublicKey and \a linkAction.
		BasicKeyLinkNotification(const Key& mainAccountPublicKey, const TAccountPublicKey& linkedPublicKey, model::LinkAction linkAction)
				: Notification(Notification_Type, sizeof(BasicKeyLinkNotification))
				, MainAccountPublicKey(mainAccountPublicKey)
				, LinkedPublicKey(linkedPublicKey)
				, LinkAction(linkAction)
		{}

	public:
		/// Main account public key.
		const Key& MainAccountPublicKey;

		/// Linked public key.
		/// \note Passed by value, due to voting key link transaction.
		TAccountPublicKey LinkedPublicKey;

		/// Link action.
		model::LinkAction LinkAction;
	};

	// endregion
}}
