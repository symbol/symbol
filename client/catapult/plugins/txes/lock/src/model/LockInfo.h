#pragma once
#include "LockTypes.h"
#include "catapult/types.h"

namespace catapult { namespace model {

	/// A lock status.
	enum class LockStatus : uint8_t {
		/// The lock is unused.
		Unused,
		/// The lock was already used.
		Used
	};

	/// A lock info.
	struct LockInfo {
	protected:
		/// Creates a default lock info.
		LockInfo()
		{}

		/// Creates a lock info around \a account, \a mosaicId, \a amount and \a height.
		explicit LockInfo(const Key& account, catapult::MosaicId mosaicId, catapult::Amount amount, catapult::Height height)
				: Account(account)
				, MosaicId(mosaicId)
				, Amount(amount)
				, Height(height)
				, Status(LockStatus::Unused)
		{}

	public:
		/// The account.
		Key Account;

		/// The mosaic id.
		catapult::MosaicId MosaicId;

		/// The amount.
		catapult::Amount Amount;

		/// The height where the lock expires.
		catapult::Height Height;

		/// Flag indicating whether or not the lock was already used.
		LockStatus Status;

	public:
		/// Returns \c true if lock info is active at \a height.
		constexpr bool isActive(catapult::Height height) const {
			return height < Height;
		}
	};

	/// A hash lock info.
	struct HashLockInfo : public LockInfo {
	public:
		/// Creates a default hash lock info.
		HashLockInfo() : LockInfo()
		{}

		/// Creates a hash lock info around \a account, \a mosaicId, \a amount, \a height and \a hash.
		HashLockInfo(
				const Key& account,
				catapult::MosaicId mosaicId,
				catapult::Amount amount,
				catapult::Height height,
				const Hash256& hash)
				: LockInfo(account, mosaicId, amount, height)
				, Hash(hash)
		{}

	public:
		/// The hash.
		Hash256 Hash;
	};

	/// A secret lock info.
	struct SecretLockInfo : public LockInfo {
	public:
		/// Creates a default secret lock info.
		SecretLockInfo() : LockInfo()
		{}

		/// Creates a secret lock info around \a account, \a mosaicId, \a amount, \a height, \a hashAlgorithm, \a secret and \a recipient.
		SecretLockInfo(
				const Key& account,
				catapult::MosaicId mosaicId,
				catapult::Amount amount,
				catapult::Height height,
				LockHashAlgorithm hashAlgorithm,
				const Hash512& secret,
				const catapult::Address& recipient)
				: LockInfo(account, mosaicId, amount, height)
				, HashAlgorithm(hashAlgorithm)
				, Secret(secret)
				, Recipient(recipient)
		{}

	public:
		/// The hash algorithm.
		LockHashAlgorithm HashAlgorithm;

		/// The secret.
		Hash512 Secret;

		/// The recipient of the locked mosaic.
		catapult::Address Recipient;
	};
}}
