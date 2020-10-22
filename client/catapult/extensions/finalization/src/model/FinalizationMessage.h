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
#include "StepIdentifier.h"
#include "catapult/crypto_voting/BmTreeSignature.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/model/TrailingVariableDataLayout.h"

namespace catapult {
	namespace crypto { class AggregateBmPrivateKeyTree; }
	namespace model { class FinalizationContext; }
}

namespace catapult { namespace model {

	// region FinalizationMessage

#pragma pack(push, 1)

	/// Finalization message payload.
	struct FinalizationMessagePayload {
		/// Message version.
		uint32_t Version;

		/// Number of hashes.
		uint32_t HashesCount;

		/// Step identifer.
		model::StepIdentifier StepIdentifier;

		/// Block height corresponding to the the first hash.
		catapult::Height Height;
	};

#define DEFINE_FINALIZATION_MESSAGE_ACCESSOR(TYPE, NAME, OFFSET) \
	/* Returns a const reference to the typed data contained in this message. */ \
	const TYPE& NAME() const { \
		return *reinterpret_cast<const TYPE*>(ToBytePointer(*this) + sizeof(FinalizationMessage) + OFFSET); \
	} \
	\
	/* Returns a reference to the typed data contained in this message. */ \
	TYPE& NAME() { \
		return *reinterpret_cast<TYPE*>(ToBytePointer(*this) + sizeof(FinalizationMessage) + OFFSET); \
	}

#define DEFINE_TRAILING_VARIABLE_DATA_LAYOUT_ACCESSORS_CUSTOM_OFFSET(NAME, SIZE, OFFSET) \
	/* Returns a const pointer to the typed data contained in this entity. */ \
	const auto* NAME##Ptr() const { \
		return SIZE && Size == CalculateRealSize(*this) \
				? ToTypedPointer(ToBytePointer(*this) + sizeof(FinalizationMessage) + OFFSET) \
				: nullptr; \
	} \
	\
	/* Returns a pointer to the typed data contained in this entity. */ \
	auto* NAME##Ptr() { \
		return SIZE && Size == CalculateRealSize(*this) \
				? ToTypedPointer(ToBytePointer(*this) + sizeof(FinalizationMessage) + OFFSET) \
				: nullptr; \
	}

	/// Finalization message.
	struct FinalizationMessage : public TrailingVariableDataLayout<FinalizationMessage, Hash256> {
	public:
		/// Size of the header that can be skipped when signing/verifying.
		static constexpr size_t Header_Size = sizeof(uint32_t) * 2 + sizeof(crypto::BmTreeSignature);

		/// Message format version.
		static constexpr uint8_t Current_Version = 1;

		/// Minimum message size (current version).
		static constexpr uint32_t MinSize() {
			return sizeof(FinalizationMessage) + sizeof(crypto::BmTreeSignature) + sizeof(FinalizationMessagePayload);
		}

	public:
		/// Reserved padding to align Signature on 8-byte boundary.
		uint16_t FinalizationMessage_Reserved1;

		/// Signature scheme.
		uint16_t SignatureScheme;

		// Message signature when signature scheme is 1
		DEFINE_FINALIZATION_MESSAGE_ACCESSOR(crypto::BmTreeSignature, Signature, 0)

		// Message signature when signature scheme is 0
		DEFINE_FINALIZATION_MESSAGE_ACCESSOR(crypto::BmTreeSignatureV1, SignatureV1, 0)

		// Message payload
		DEFINE_FINALIZATION_MESSAGE_ACCESSOR(FinalizationMessagePayload, Data, SignatureSize())

		/// Hashes.
		DEFINE_TRAILING_VARIABLE_DATA_LAYOUT_ACCESSORS_CUSTOM_OFFSET(
				Hashes,
				Data().HashesCount,
				SignatureSize() + sizeof(FinalizationMessagePayload))

	public:
		/// Calculates the real size of \a message.
		static uint64_t CalculateRealSize(const FinalizationMessage& message) noexcept {
			return sizeof(FinalizationMessage) + message.SignatureSize() + sizeof(FinalizationMessagePayload)
					+ message.Data().HashesCount * Hash256::Size;
		}

	private:
		constexpr size_t SignatureSize() const {
			return 1 == SignatureScheme ? sizeof(crypto::BmTreeSignature) : sizeof(crypto::BmTreeSignatureV1);
		}

	private:
		static const uint8_t* ToBytePointer(const FinalizationMessage& derived) {
			return reinterpret_cast<const uint8_t*>(&derived);
		}

		static uint8_t* ToBytePointer(FinalizationMessage& derived) {
			return reinterpret_cast<uint8_t*>(&derived);
		}
	};

#pragma pack(pop)

	/// Insertion operator for outputting \a message to \a out.
	std::ostream& operator<<(std::ostream& out, const FinalizationMessage& message);

	/// Range of finalization messages.
	using FinalizationMessageRange = EntityRange<FinalizationMessage>;

	// endregion

	// region CalculateMessageHash

	/// Calculates a hash for \a message.
	Hash256 CalculateMessageHash(const FinalizationMessage& message);

	// endregion

	// region PrepareMessage

	/// Prepares a finalization message given \a bmPrivateKeyTree, \a stepIdentifier, \a height and \a hashes.
	std::unique_ptr<FinalizationMessage> PrepareMessage(
			crypto::AggregateBmPrivateKeyTree& bmPrivateKeyTree,
			const StepIdentifier& stepIdentifier,
			Height height,
			const HashRange& hashes);

	// endregion

	// region ProcessMessage

#define PROCESS_MESSAGE_RESULT_LIST \
	/* Invalid message signature. */ \
	ENUM_VALUE(Failure_Signature) \
	\
	/* Invalid padding. */ \
	ENUM_VALUE(Failure_Padding) \
	\
	/* Invalid version. */ \
	ENUM_VALUE(Failure_Version) \
	\
	/* Invalid voter. */ \
	ENUM_VALUE(Failure_Voter) \
	\
	/* Processing succeeded. */ \
	ENUM_VALUE(Success)

#define ENUM_VALUE(LABEL) LABEL,
	/// Process message results.
	enum class ProcessMessageResult {
		PROCESS_MESSAGE_RESULT_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, ProcessMessageResult value);

	/// Processes a finalization \a message using \a context.
	std::pair<ProcessMessageResult, size_t> ProcessMessage(const FinalizationMessage& message, const FinalizationContext& context);

	// endregion
}}
