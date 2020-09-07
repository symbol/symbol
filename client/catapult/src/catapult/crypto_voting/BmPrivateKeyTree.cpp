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

#include "BmPrivateKeyTree.h"
#include "catapult/crypto/SecureRandomGenerator.h"
#include "catapult/crypto/Signer.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/exceptions.h"
#include <type_traits>

namespace catapult { namespace crypto {

	namespace {
		using BmSignature = decltype(BmTreeSignature::Root.Signature);
		using BmPublicKey = decltype(BmTreeSignature::Root.ParentPublicKey);
		using BmPrivateKey = PrivateKey;
		using BmKeyPair = KeyPair;

		// region signed key pair

		BmPrivateKey GeneratePrivateKey() {
			SecureRandomGenerator generator;
			return BmPrivateKey::Generate([&generator]() { return static_cast<uint8_t>(generator()); });
		}

		BmPublicKey GetPublicKey(const BmPrivateKey& privateKey) {
			auto iter = privateKey.begin();
			auto keyPair = BmKeyPair::FromPrivate(BmPrivateKey::Generate([&iter]{
				return *iter++;
			}));
			return keyPair.publicKey();
		}

		RawBuffer ToBuffer(const uint64_t& value) {
			return { reinterpret_cast<const uint8_t*>(&value), sizeof(uint64_t) };
		}

		class SignedPrivateKey {
		public:
			static constexpr auto Entry_Size = sizeof(BmPrivateKey) + BmSignature::Size;

		public:
			SignedPrivateKey(BmPrivateKey&& privateKey, const BmSignature& signature)
					: m_privateKey(std::move(privateKey))
					, m_signature(signature)
			{}

		public:
			static SignedPrivateKey CreateRandom(const BmKeyPair& parentKeyPair, uint64_t identifier) {
				auto privateKey = GeneratePrivateKey();
				auto publicKey = GetPublicKey(privateKey);

				BmSignature signature;
				Sign(parentKeyPair, { publicKey, ToBuffer(identifier) }, signature);
				return SignedPrivateKey(std::move(privateKey), signature);
			}

		public:
			const BmPrivateKey& privateKey() const {
				return m_privateKey;
			}

			const BmSignature& signature() const {
				return m_signature;
			}

			BmPrivateKey&& detachPrivateKey() {
				return std::move(m_privateKey);
			}

		private:
			BmPrivateKey m_privateKey;
			BmSignature m_signature;
		};

		SignedPrivateKey ReadSignedPrivateKey(io::InputStream& input) {
			auto privateKey = BmPrivateKey::Generate([&input]() { return io::Read8(input); });
			BmSignature signature;
			input.read(signature);

			return SignedPrivateKey(std::move(privateKey), signature);
		}

		void Write(io::OutputStream& output, const SignedPrivateKey& signedPrivateKey) {
			output.write(signedPrivateKey.privateKey());
			output.write(signedPrivateKey.signature());
		}

		void WipeSignedPrivateKey(io::OutputStream& output) {
			auto buffer = std::array<uint8_t, BmPrivateKey::Size>();
			output.write(buffer);
		}

		// endregion
	}

	// region level

	class BmPrivateKeyTree::Level {
	private:
		Level(
				const BmPublicKey& parentPublicKey,
				uint64_t startIdentifier,
				uint64_t endIdentifier,
				std::vector<SignedPrivateKey>&& signedPrivateKeys)
				: m_parentPublicKey(parentPublicKey)
				, m_startIdentifier(startIdentifier)
				, m_endIdentifier(endIdentifier)
				, m_levelSignedPrivateKeys(std::move(signedPrivateKeys))
		{}

	public:
		static Level FromStream(io::InputStream& input) {
			BmPublicKey parentPublicKey;
			input.read(parentPublicKey);
			auto startIdentifier = io::Read64(input);
			auto endIdentifier = io::Read64(input);

			std::vector<SignedPrivateKey> signedPrivateKeys;
			for (auto i = 0u; i <= endIdentifier - startIdentifier; ++i)
				signedPrivateKeys.push_back(ReadSignedPrivateKey(input));

			return Level(parentPublicKey, startIdentifier, endIdentifier, std::move(signedPrivateKeys));
		}

		static Level Create(BmKeyPair&& keyPair, uint64_t startIdentifier, uint64_t endIdentifier) {
			std::vector<SignedPrivateKey> signedPrivateKeys;
			for (auto i = 0u; i <= endIdentifier - startIdentifier; ++i)
				signedPrivateKeys.push_back(SignedPrivateKey::CreateRandom(keyPair, endIdentifier - i));

			return Level(keyPair.publicKey(), startIdentifier, endIdentifier, std::move(signedPrivateKeys));
		}

	public:
		const BmPublicKey& publicKey() const {
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

		BmTreeSignature::ParentPublicKeySignaturePair publicKeySignature(uint64_t identifier) const {
			auto index = m_endIdentifier - identifier;
			return { m_parentPublicKey, m_levelSignedPrivateKeys[index].signature() };
		}

		void wipeUntil(uint64_t identifier) {
			auto index = m_endIdentifier - identifier;
			while (m_levelSignedPrivateKeys.size() > index + 1)
				m_levelSignedPrivateKeys.pop_back();
		}

		BmPrivateKey&& detachPrivateKey(uint64_t identifier) {
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
		BmPublicKey m_parentPublicKey;
		uint64_t m_startIdentifier;
		uint64_t m_endIdentifier;
		std::vector<SignedPrivateKey> m_levelSignedPrivateKeys;
	};

	// endregion

	// region bm private key tree

	namespace {
		enum : size_t {
			Layer_Top,
			Layer_Low
		};

		constexpr auto Tree_Header_Size = sizeof(BmOptions) + sizeof(BmKeyIdentifier);
		constexpr auto Layer_Header_Size = BmPublicKey::Size + sizeof(uint64_t) + sizeof(uint64_t);
		constexpr auto Invalid_Batch_Id = 0xFFFF'FFFF'FFFF'FFFF;

		uint64_t IndexToOffset(uint64_t index) {
			return Layer_Header_Size + index * SignedPrivateKey::Entry_Size;
		}

		BmKeyIdentifier LoadKeyIdentifier(io::InputStream& input) {
			BmKeyIdentifier keyIdentifier;
			keyIdentifier.BatchId = io::Read64(input);
			keyIdentifier.KeyId = io::Read64(input);
			return keyIdentifier;
		}

		BmOptions LoadOptions(io::InputStream& input) {
			BmOptions options;
			options.Dilution = io::Read64(input);
			options.StartKeyIdentifier = LoadKeyIdentifier(input);
			options.EndKeyIdentifier = LoadKeyIdentifier(input);
			return options;
		}

		void SaveKeyIdentifier(io::OutputStream& output, const BmKeyIdentifier& keyIdentifier) {
			io::Write64(output, keyIdentifier.BatchId);
			io::Write64(output, keyIdentifier.KeyId);
		}

		void SaveOptions(io::OutputStream& output, const BmOptions& options) {
			io::Write64(output, options.Dilution);
			SaveKeyIdentifier(output, options.StartKeyIdentifier);
			SaveKeyIdentifier(output, options.EndKeyIdentifier);
		}

		void SaveHeader(io::OutputStream& output, const BmOptions& options, const BmKeyIdentifier& lastKeyIdentifier) {
			SaveOptions(output, options);
			SaveKeyIdentifier(output, lastKeyIdentifier);
		}
	}

	BmPrivateKeyTree::BmPrivateKeyTree(io::SeekableStream& storage, const BmOptions& options)
			: m_storage(storage)
			, m_options(options)
			, m_lastKeyIdentifier({ Invalid_Batch_Id, 0 })
	{}

	BmPrivateKeyTree::BmPrivateKeyTree(BmPrivateKeyTree&&) = default;

	BmPrivateKeyTree BmPrivateKeyTree::FromStream(io::SeekableStream& storage) {
		auto options = LoadOptions(storage);
		BmPrivateKeyTree tree(storage, options);

		// FromStream loads whole level, used keys are zeroed. wipeUntil() is used to have consistent view.
		tree.m_lastKeyIdentifier = LoadKeyIdentifier(storage);

		tree.m_levels[Layer_Top] = std::make_unique<Level>(Level::FromStream(storage));

		// if any sign() was issued prior to saving, load subsequent levels
		if (Invalid_Batch_Id != tree.m_lastKeyIdentifier.BatchId) {
			tree.m_levels[Layer_Top]->wipeUntil(tree.m_lastKeyIdentifier.BatchId);

			tree.m_levels[Layer_Low] = std::make_unique<Level>(Level::FromStream(storage));
			tree.m_levels[Layer_Low]->wipeUntil(tree.m_lastKeyIdentifier.KeyId);
		}

		return tree;
	}

	BmPrivateKeyTree BmPrivateKeyTree::Create(BmKeyPair&& keyPair, io::SeekableStream& storage, const BmOptions& options) {
		BmPrivateKeyTree tree(storage, options);
		SaveHeader(tree.m_storage, tree.m_options, tree.m_lastKeyIdentifier);
		tree.createLevel(Layer_Top, std::move(keyPair), options.StartKeyIdentifier.BatchId, options.EndKeyIdentifier.BatchId);
		return tree;
	}

	BmPrivateKeyTree::~BmPrivateKeyTree() = default;

	const BmPublicKey& BmPrivateKeyTree::rootPublicKey() const {
		return m_levels[Layer_Top]->publicKey();
	}

	const BmOptions& BmPrivateKeyTree::options() const {
		return m_options;
	}

	bool BmPrivateKeyTree::canSign(const BmKeyIdentifier& keyIdentifier) const {
		if (Invalid_Batch_Id != m_lastKeyIdentifier.BatchId && keyIdentifier <= m_lastKeyIdentifier)
			return false;

		if (keyIdentifier < m_options.StartKeyIdentifier || keyIdentifier > m_options.EndKeyIdentifier)
			return false;

		if (keyIdentifier.KeyId >= m_options.Dilution)
			return false;

		return true;
	}

	BmTreeSignature BmPrivateKeyTree::sign(const BmKeyIdentifier& keyIdentifier, const RawBuffer& dataBuffer) {
		if (!canSign(keyIdentifier))
			CATAPULT_THROW_RUNTIME_ERROR_1("sign called with invalid key identifier", keyIdentifier);

		if (m_lastKeyIdentifier.BatchId != keyIdentifier.BatchId) {
			auto endKeyId = m_options.EndKeyIdentifier.BatchId == keyIdentifier.BatchId
					? m_options.EndKeyIdentifier.KeyId
					: m_options.Dilution - 1;
			createLevel(Layer_Low, detachKeyPair(Layer_Top, keyIdentifier.BatchId), keyIdentifier.KeyId, endKeyId);
		}

		auto subKeyPair = detachKeyPair(Layer_Low, keyIdentifier.KeyId);
		BmSignature msgSignature;
		crypto::Sign(subKeyPair, dataBuffer, msgSignature);

		m_lastKeyIdentifier = keyIdentifier;
		m_storage.seek(sizeof(BmOptions));
		SaveKeyIdentifier(m_storage, m_lastKeyIdentifier);

		return {
			m_levels[Layer_Top]->publicKeySignature(keyIdentifier.BatchId),
			m_levels[Layer_Low]->publicKeySignature(keyIdentifier.KeyId),
			{ subKeyPair.publicKey(), msgSignature }
		};
	}

	size_t BmPrivateKeyTree::levelOffset(size_t depth) const {
		auto offset = Tree_Header_Size;
		for (size_t i = 0; i < depth; ++i)
			offset += IndexToOffset(m_levels[i]->endIdentifier() - m_levels[i]->startIdentifier() + 1);

		return offset;
	}

	BmKeyPair BmPrivateKeyTree::detachKeyPair(size_t depth, uint64_t identifier) {
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
		return BmKeyPair::FromPrivate(level.detachPrivateKey(identifier));
	}

	void BmPrivateKeyTree::createLevel(size_t depth, BmKeyPair&& keyPair, uint64_t startIdentifier, uint64_t endIdentifier) {
		auto offset = levelOffset(depth);
		m_levels[depth] = std::make_unique<Level>(Level::Create(std::move(keyPair), startIdentifier, endIdentifier));

		m_storage.seek(offset);
		m_levels[depth]->write(m_storage);
	}

	// endregion

	namespace {
		bool VerifyBoundSignature(
				const BmTreeSignature::ParentPublicKeySignaturePair& pair,
				const BmPublicKey& signedPublicKey,
				uint64_t boundary) {
			return crypto::Verify(pair.ParentPublicKey, { signedPublicKey, ToBuffer(boundary) }, pair.Signature);
		}
	}

	bool Verify(const BmTreeSignature& signature, const BmKeyIdentifier& keyIdentifier, const RawBuffer& buffer) {
		if (!VerifyBoundSignature(signature.Root, signature.Top.ParentPublicKey, keyIdentifier.BatchId))
			return false;

		if (!VerifyBoundSignature(signature.Top, signature.Bottom.ParentPublicKey, keyIdentifier.KeyId))
			return false;

		if (!crypto::Verify(signature.Bottom.ParentPublicKey, buffer, signature.Bottom.Signature))
			return false;

		return true;
	}
}}
