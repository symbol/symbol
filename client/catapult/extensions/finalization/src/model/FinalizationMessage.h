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

	/// Finalization message.
	struct FinalizationMessage : public TrailingVariableDataLayout<FinalizationMessage, Hash256> {
	public:
		/// Size of the header that can be skipped when signing/verifying.
		static constexpr size_t Header_Size = sizeof(uint32_t) * 2 + sizeof(crypto::BmTreeSignature);

		/// Message format version.
		static constexpr uint8_t Current_Version = 1;

	public:
		/// Reserved padding to align Signature on 8-byte boundary.
		uint32_t FinalizationMessage_Reserved1;

		/// Message signature.
		crypto::BmTreeSignature Signature;

		/// Message version.
		uint32_t Version;

		/// Number of hashes.
		uint32_t HashesCount;

		/// Step identifer.
		model::StepIdentifier StepIdentifier;

		/// Block height corresponding to the the first hash.
		catapult::Height Height;

	public:
		DEFINE_TRAILING_VARIABLE_DATA_LAYOUT_ACCESSORS(Hashes, Count)

	public:
		/// Calculates the real size of \a message.
		static constexpr uint64_t CalculateRealSize(const FinalizationMessage& message) noexcept {
			return sizeof(FinalizationMessage) + message.HashesCount * Hash256::Size;
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
