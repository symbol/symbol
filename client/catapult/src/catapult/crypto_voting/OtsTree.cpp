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
#include "catapult/crypto/SecureRandomGenerator.h"
#include "catapult/crypto/Signer.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/exceptions.h"
#include <type_traits>

namespace catapult { namespace crypto {

	namespace {
		using OtsPrivateKeyType = PrivateKey;

		// region signed key pair

		OtsPrivateKeyType GeneratePrivateKey() {
			SecureRandomGenerator generator;
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
			Layer_Low
		};

		constexpr auto Tree_Header_Size = sizeof(OtsOptions) + sizeof(OtsKeyIdentifier);
		constexpr auto Layer_Header_Size = OtsPublicKey::Size + sizeof(uint64_t) + sizeof(uint64_t);
		constexpr auto Invalid_Batch_Id = 0xFFFF'FFFF'FFFF'FFFF;

		uint64_t IndexToOffset(uint64_t index) {
			return Layer_Header_Size + index * SignedPrivateKey::Entry_Size;
		}

		OtsKeyIdentifier LoadKeyIdentifier(io::InputStream& input) {
			OtsKeyIdentifier keyIdentifier;
			keyIdentifier.BatchId = io::Read64(input);
			keyIdentifier.KeyId = io::Read64(input);
			return keyIdentifier;
		}

		OtsOptions LoadOptions(io::InputStream& input) {
			OtsOptions options;
			options.Dilution = io::Read64(input);
			options.StartKeyIdentifier = LoadKeyIdentifier(input);
			options.EndKeyIdentifier = LoadKeyIdentifier(input);
			return options;
		}

		void SaveKeyIdentifier(io::OutputStream& output, const OtsKeyIdentifier& keyIdentifier) {
			io::Write64(output, keyIdentifier.BatchId);
			io::Write64(output, keyIdentifier.KeyId);
		}

		void SaveOptions(io::OutputStream& output, const OtsOptions& options) {
			io::Write64(output, options.Dilution);
			SaveKeyIdentifier(output, options.StartKeyIdentifier);
			SaveKeyIdentifier(output, options.EndKeyIdentifier);
		}

		void SaveHeader(io::OutputStream& output, const OtsOptions& options, const OtsKeyIdentifier& lastKeyIdentifier) {
			SaveOptions(output, options);
			SaveKeyIdentifier(output, lastKeyIdentifier);
		}
	}

	OtsTree::OtsTree(io::SeekableStream& storage, const OtsOptions& options)
			: m_storage(storage)
			, m_options(options)
			, m_lastKeyIdentifier({ Invalid_Batch_Id, 0 })
	{}

	OtsTree::OtsTree(OtsTree&&) = default;

	OtsTree OtsTree::FromStream(io::SeekableStream& storage) {
		auto options = LoadOptions(storage);
		OtsTree tree(storage, options);

		// FromStream loads whole level, used keys are zeroed. wipeUntil() is used to have consistent view.
		tree.m_lastKeyIdentifier = LoadKeyIdentifier(storage);

		tree.m_levels[Layer_Top] = std::make_unique<OtsLevel>(OtsLevel::FromStream(storage));

		// if any sign() was issued prior to saving, load subsequent levels
		if (Invalid_Batch_Id != tree.m_lastKeyIdentifier.BatchId) {
			tree.m_levels[Layer_Top]->wipeUntil(tree.m_lastKeyIdentifier.BatchId);

			tree.m_levels[Layer_Low] = std::make_unique<OtsLevel>(OtsLevel::FromStream(storage));
			tree.m_levels[Layer_Low]->wipeUntil(tree.m_lastKeyIdentifier.KeyId);
		}

		return tree;
	}

	OtsTree OtsTree::Create(OtsKeyPairType&& keyPair, io::SeekableStream& storage, const OtsOptions& options) {
		OtsTree tree(storage, options);
		SaveHeader(tree.m_storage, tree.m_options, tree.m_lastKeyIdentifier);
		tree.createLevel(Layer_Top, std::move(keyPair), options.StartKeyIdentifier.BatchId, options.EndKeyIdentifier.BatchId);
		return tree;
	}

	OtsTree::~OtsTree() = default;

	const OtsPublicKey& OtsTree::rootPublicKey() const {
		return m_levels[Layer_Top]->publicKey();
	}

	const OtsOptions& OtsTree::options() const {
		return m_options;
	}

	bool OtsTree::canSign(const OtsKeyIdentifier& keyIdentifier) const {
		if (Invalid_Batch_Id != m_lastKeyIdentifier.BatchId && keyIdentifier <= m_lastKeyIdentifier)
			return false;

		if (keyIdentifier < m_options.StartKeyIdentifier || keyIdentifier > m_options.EndKeyIdentifier)
			return false;

		if (keyIdentifier.KeyId >= m_options.Dilution)
			return false;

		return true;
	}

	OtsTreeSignature OtsTree::sign(const OtsKeyIdentifier& keyIdentifier, const RawBuffer& dataBuffer) {
		if (!canSign(keyIdentifier))
			CATAPULT_THROW_RUNTIME_ERROR_1("sign called with invalid key identifier", keyIdentifier);

		if (m_lastKeyIdentifier.BatchId != keyIdentifier.BatchId) {
			auto endKeyId = m_options.EndKeyIdentifier.BatchId == keyIdentifier.BatchId
					? m_options.EndKeyIdentifier.KeyId
					: m_options.Dilution - 1;
			createLevel(Layer_Low, detachKeyPair(Layer_Top, keyIdentifier.BatchId), keyIdentifier.KeyId, endKeyId);
		}

		auto subKeyPair = detachKeyPair(Layer_Low, keyIdentifier.KeyId);
		OtsSignature msgSignature;
		crypto::Sign(subKeyPair, dataBuffer, msgSignature);

		m_lastKeyIdentifier = keyIdentifier;
		m_storage.seek(sizeof(OtsOptions));
		SaveKeyIdentifier(m_storage, m_lastKeyIdentifier);

		return {
			m_levels[Layer_Top]->publicKeySignature(keyIdentifier.BatchId),
			m_levels[Layer_Low]->publicKeySignature(keyIdentifier.KeyId),
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

	bool Verify(const OtsTreeSignature& signature, const OtsKeyIdentifier& keyIdentifier, const RawBuffer& buffer) {
		if (!VerifyBoundSignature(signature.Root, signature.Top.ParentPublicKey, keyIdentifier.BatchId))
			return false;

		if (!VerifyBoundSignature(signature.Top, signature.Bottom.ParentPublicKey, keyIdentifier.KeyId))
			return false;

		if (!crypto::Verify(signature.Bottom.ParentPublicKey, buffer, signature.Bottom.Signature))
			return false;

		return true;
	}
}}
