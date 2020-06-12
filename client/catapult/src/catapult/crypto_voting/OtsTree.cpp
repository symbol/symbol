/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "OtsTree.h"
#include "catapult/crypto/Signer.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/utils/RandomGenerator.h"
#include "catapult/exceptions.h"
#include <type_traits>

namespace catapult { namespace crypto {

	namespace {
		using OtsPrivateKeyType = PrivateKey;

		// region signed key pair

		OtsPrivateKeyType GeneratePrivateKey() {
			// TODO: consider using openssl-based generator instead.
			utils::HighEntropyRandomGenerator generator;
			return OtsPrivateKeyType::Generate([&generator]() { return static_cast<uint8_t>(generator()); });
		}

		OtsPublicKey GetPublicKey(const OtsPrivateKeyType& privateKey) {
			auto iter = privateKey.begin();
			auto keyPair = OtsKeyPairType::FromPrivate(OtsPrivateKeyType::Generate([&iter]{
				return *iter++;
			}));
			return keyPair.publicKey();
		}

		RawBuffer ToBuffer(const uint64_t& value) {
			return { reinterpret_cast<const uint8_t*>(&value), sizeof(uint64_t) };
		}

		class SignedPrivateKey {
		public:
			static constexpr auto Entry_Size = sizeof(OtsPrivateKeyType) + OtsSignature::Size;

		public:
			SignedPrivateKey(OtsPrivateKeyType&& privateKey, const OtsSignature& signature)
					: m_privateKey(std::move(privateKey))
					, m_signature(signature)
			{}

		public:
			static SignedPrivateKey CreateRandom(const OtsKeyPairType& parentKeyPair, uint64_t identifier) {
				auto privateKey = GeneratePrivateKey();
				auto publicKey = GetPublicKey(privateKey);

				OtsSignature signature;
				Sign(parentKeyPair, { publicKey, ToBuffer(identifier) }, signature);
				return SignedPrivateKey(std::move(privateKey), signature);
			}

		public:
			const OtsPrivateKeyType& privateKey() const {
				return m_privateKey;
			}

			const OtsSignature& signature() const {
				return m_signature;
			}

			OtsPrivateKeyType&& detachPrivateKey() {
				return std::move(m_privateKey);
			}

		private:
			OtsPrivateKeyType m_privateKey;
			OtsSignature m_signature;
		};

		SignedPrivateKey ReadSignedPrivateKey(io::InputStream& input) {
			auto privateKey = OtsPrivateKeyType::Generate([&input]() { return io::Read8(input); });
			OtsSignature signature;
			input.read(signature);

			return SignedPrivateKey(std::move(privateKey), signature);
		}

		void Write(io::OutputStream& output, const SignedPrivateKey& signedPrivateKey) {
			output.write(signedPrivateKey.privateKey());
			output.write(signedPrivateKey.signature());
		}

		void WipeSignedPrivateKey(io::OutputStream& output) {
			auto buffer = std::array<uint8_t, OtsPrivateKeyType::Size>();
			output.write(buffer);
		}

		// endregion
	}

	// region ots level

	class OtsTree::OtsLevel {
	private:
		OtsLevel(
				const OtsPublicKey& parentPublicKey,
				uint64_t startIdentifier,
				uint64_t endIdentifier,
				std::vector<SignedPrivateKey>&& signedPrivateKeys)
				: m_parentPublicKey(parentPublicKey)
				, m_startIdentifier(startIdentifier)
				, m_endIdentifier(endIdentifier)
				, m_levelSignedPrivateKeys(std::move(signedPrivateKeys))
		{}

	public:
		static OtsLevel FromStream(io::InputStream& input) {
			OtsPublicKey parentPublicKey;
			input.read(parentPublicKey);
			auto startIdentifier = io::Read64(input);
			auto endIdentifier = io::Read64(input);

			std::vector<SignedPrivateKey> signedPrivateKeys;
			for (auto i = 0u; i <= endIdentifier - startIdentifier; ++i)
				signedPrivateKeys.push_back(ReadSignedPrivateKey(input));

			return OtsLevel(parentPublicKey, startIdentifier, endIdentifier, std::move(signedPrivateKeys));
		}

		static OtsLevel Create(OtsKeyPairType&& keyPair, uint64_t startIdentifier, uint64_t endIdentifier) {
			std::vector<SignedPrivateKey> signedPrivateKeys;
			for (auto i = 0u; i <= endIdentifier - startIdentifier; ++i)
				signedPrivateKeys.push_back(SignedPrivateKey::CreateRandom(keyPair, endIdentifier - i));

			return OtsLevel(keyPair.publicKey(), startIdentifier, endIdentifier, std::move(signedPrivateKeys));
		}

	public:
		const OtsPublicKey& publicKey() const {
			return m_parentPublicKey;
		}

		uint64_t startIdentifier() const {
			return m_startIdentifier;
		}

		uint64_t endIdentifier() const {
			return m_endIdentifier;
		}

		size_t size() const {
			return m_levelSignedPrivateKeys.size();
		}

		OtsParentPublicKeySignaturePair publicKeySignature(uint64_t identifier) const {
			auto index = m_endIdentifier - identifier;
			return { m_parentPublicKey, m_levelSignedPrivateKeys[index].signature() };
		}

		void wipeUntil(uint64_t identifier) {
			auto index = m_endIdentifier - identifier;
			while (m_levelSignedPrivateKeys.size() > index + 1)
				m_levelSignedPrivateKeys.pop_back();
		}

		OtsPrivateKeyType&& detachPrivateKey(uint64_t identifier) {
			auto index = m_endIdentifier - identifier;
			return std::move(m_levelSignedPrivateKeys[index].detachPrivateKey());
		}

		void write(io::OutputStream& output) const {
			output.write(m_parentPublicKey);
			io::Write64(output, m_startIdentifier);
			io::Write64(output, m_endIdentifier);

			for (const auto& signedPrivateKey : m_levelSignedPrivateKeys)
				Write(output, signedPrivateKey);
		}

	private:
		OtsPublicKey m_parentPublicKey;
		uint64_t m_startIdentifier;
		uint64_t m_endIdentifier;
		std::vector<SignedPrivateKey> m_levelSignedPrivateKeys;
	};

	// endregion

	// region ots tree

	namespace {
		enum : size_t {
			Layer_Top,
			Layer_Mid,
			Layer_Low
		};

		constexpr auto Tree_Header_Size = sizeof(OtsOptions) + sizeof(StepIdentifier);
		constexpr auto Layer_Header_Size = OtsPublicKey::Size + sizeof(uint64_t) + sizeof(uint64_t);

		uint64_t IndexToOffset(uint64_t index) {
			return Layer_Header_Size + index * SignedPrivateKey::Entry_Size;
		}

		OtsOptions LoadOptions(io::InputStream& input) {
			OtsOptions options;
			options.MaxRounds = io::Read64(input);
			options.MaxSubRounds = io::Read64(input);
			return options;
		}

		StepIdentifier LoadStep(io::InputStream& input) {
			StepIdentifier stepIdentifier;
			stepIdentifier.Point = io::Read64(input);
			stepIdentifier.Round = io::Read64(input);
			stepIdentifier.SubRound = io::Read64(input);
			return stepIdentifier;
		}

		void SaveOptions(io::OutputStream& output, const OtsOptions& options) {
			io::Write64(output, options.MaxRounds);
			io::Write64(output, options.MaxSubRounds);
		}

		void SaveStep(io::OutputStream& output, const StepIdentifier& stepIdentifier) {
			io::Write64(output, stepIdentifier.Point);
			io::Write64(output, stepIdentifier.Round);
			io::Write64(output, stepIdentifier.SubRound);
		}

		void SaveHeader(io::OutputStream& output, const OtsOptions& options, const StepIdentifier& stepIdentifier) {
			SaveOptions(output, options);
			SaveStep(output, stepIdentifier);
		}
	}

	OtsTree::OtsTree(SeekableOutputStream& storage, const OtsOptions& options)
			: m_storage(storage)
			, m_options(options)
			, m_lastStep()
	{}

	OtsTree::OtsTree(OtsTree&&) = default;

	OtsTree OtsTree::FromStream(io::InputStream& input, SeekableOutputStream& storage) {
		OtsTree tree(storage, LoadOptions(input));

		// FromStream loads whole level, used keys are zeroed. wipeUntil() is used to have consistent view.
		tree.m_lastStep = LoadStep(input);
		tree.m_levels[Layer_Top] = std::make_unique<OtsLevel>(OtsLevel::FromStream(input));
		tree.m_levels[Layer_Top]->wipeUntil(tree.m_lastStep.Point);

		// if any sign() was issued prior to saving, load subsequent levels
		if (tree.m_lastStep.Point) {
			tree.m_levels[Layer_Mid] = std::make_unique<OtsLevel>(OtsLevel::FromStream(input));
			tree.m_levels[Layer_Mid]->wipeUntil(tree.m_lastStep.Round);
			tree.m_levels[Layer_Low] = std::make_unique<OtsLevel>(OtsLevel::FromStream(input));
			tree.m_levels[Layer_Low]->wipeUntil(tree.m_lastStep.SubRound);
		}

		return tree;
	}

	OtsTree OtsTree::Create(
			OtsKeyPairType&& keyPair,
			SeekableOutputStream& storage,
			FinalizationPoint startPoint,
			FinalizationPoint endPoint,
			const OtsOptions& options) {
		OtsTree tree(storage, options);
		SaveHeader(tree.m_storage, tree.m_options, tree.m_lastStep);
		tree.createLevel(Layer_Top, std::move(keyPair), startPoint.unwrap(), endPoint.unwrap());
		return tree;
	}

	OtsTree::~OtsTree() = default;

	const OtsPublicKey& OtsTree::rootPublicKey() const {
		return m_levels[Layer_Top]->publicKey();
	}

	bool OtsTree::canSign(const StepIdentifier& stepIdentifier) const {
		if (0 != m_lastStep.Point && stepIdentifier <= m_lastStep)
			return false;

		if (stepIdentifier.Point < m_levels[Layer_Top]->startIdentifier() || stepIdentifier.Point > m_levels[Layer_Top]->endIdentifier())
			return false;

		if (stepIdentifier.Round >= m_options.MaxRounds)
			return false;

		if (stepIdentifier.SubRound >= m_options.MaxSubRounds)
			return false;

		return true;
	}

	OtsTreeSignature OtsTree::sign(const StepIdentifier& stepIdentifier, const RawBuffer& dataBuffer) {
		if (!canSign(stepIdentifier))
			CATAPULT_THROW_RUNTIME_ERROR_1("sign called with invalid step identifier", stepIdentifier);

		if (m_lastStep.Point != stepIdentifier.Point) {
			createLevel(Layer_Mid, detachKeyPair(Layer_Top, stepIdentifier.Point), 0, m_options.MaxRounds - 1);
			createLevel(Layer_Low, detachKeyPair(Layer_Mid, stepIdentifier.Round), 0, m_options.MaxSubRounds - 1);
		} else if (m_lastStep.Round != stepIdentifier.Round) {
			createLevel(Layer_Low, detachKeyPair(Layer_Mid, stepIdentifier.Round), 0, m_options.MaxSubRounds - 1);
		}

		auto subKeyPair = detachKeyPair(Layer_Low, stepIdentifier.SubRound);
		OtsSignature msgSignature;
		crypto::Sign(subKeyPair, dataBuffer, msgSignature);

		m_lastStep = stepIdentifier;
		m_storage.seek(sizeof(OtsOptions));
		SaveStep(m_storage, m_lastStep);

		return {
			m_levels[Layer_Top]->publicKeySignature(stepIdentifier.Point),
			m_levels[Layer_Mid]->publicKeySignature(stepIdentifier.Round),
			m_levels[Layer_Low]->publicKeySignature(stepIdentifier.SubRound),
			{ subKeyPair.publicKey(), msgSignature }
		};
	}

	size_t OtsTree::levelOffset(size_t depth) const {
		auto offset = Tree_Header_Size;
		for (size_t i = 0; i < depth; ++i)
			offset += IndexToOffset(m_levels[i]->endIdentifier() - m_levels[i]->startIdentifier() + 1);

		return offset;
	}

	OtsKeyPairType OtsTree::detachKeyPair(size_t depth, uint64_t identifier) {
		auto& level = *m_levels[depth];

		// wipe keys from storage
		auto levelStartOffset = levelOffset(depth);
		auto index = level.endIdentifier() - identifier;
		for (auto i = index + 1; i < level.size(); ++i) {
			m_storage.seek(levelStartOffset + IndexToOffset(i));
			WipeSignedPrivateKey(m_storage);
		}

		// wipe from memory
		level.wipeUntil(identifier);

		// wipe requested key from storage
		m_storage.seek(levelStartOffset + IndexToOffset(index));
		WipeSignedPrivateKey(m_storage);

		// detach requested key
		return OtsKeyPairType::FromPrivate(level.detachPrivateKey(identifier));
	}

	void OtsTree::createLevel(size_t depth, KeyPair&& keyPair, uint64_t startIdentifier, uint64_t endIdentifier) {
		auto offset = levelOffset(depth);
		m_levels[depth] = std::make_unique<OtsLevel>(OtsLevel::Create(std::move(keyPair), startIdentifier, endIdentifier));

		m_storage.seek(offset);
		m_levels[depth]->write(m_storage);
	}

	// endregion

	namespace {
		bool VerifyBoundSignature(const OtsParentPublicKeySignaturePair& pair, const OtsPublicKey& signedPublicKey, uint64_t boundary) {
			return crypto::Verify(pair.ParentPublicKey, { signedPublicKey, ToBuffer(boundary) }, pair.Signature);
		}
	}

	bool Verify(const OtsTreeSignature& signature, const StepIdentifier& stepIdentifier, const RawBuffer& buffer) {
		if (!VerifyBoundSignature(signature.Root, signature.Top.ParentPublicKey, stepIdentifier.Point))
			return false;

		if (!VerifyBoundSignature(signature.Top, signature.Middle.ParentPublicKey, stepIdentifier.Round))
			return false;

		if (!VerifyBoundSignature(signature.Middle, signature.Bottom.ParentPublicKey, stepIdentifier.SubRound))
			return false;

		if (!crypto::Verify(signature.Bottom.ParentPublicKey, buffer, signature.Bottom.Signature))
			return false;

		return true;
	}
}}
