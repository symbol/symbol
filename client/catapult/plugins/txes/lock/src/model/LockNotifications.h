#pragma once
#include "LockTypes.h"
#include "catapult/model/Mosaic.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region lock notification types

/// Defines a lock notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_LOCK_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Lock, DESCRIPTION, CODE)

	/// Lock hash duration.
	DEFINE_LOCK_NOTIFICATION(Hash_Duration, 0x0001, Validator);

	/// Lock secret duration.
	DEFINE_LOCK_NOTIFICATION(Secret_Duration, 0x0002, Validator);

	/// Lock mosaic.
	DEFINE_LOCK_NOTIFICATION(Mosaic, 0x0003, Validator);

	/// Lock hash algorithm.
	DEFINE_LOCK_NOTIFICATION(Hash_Algorithm, 0x0004, Validator);

	/// Lock secret.
	DEFINE_LOCK_NOTIFICATION(Secret, 0x0005, All);

	/// Lock hash.
	DEFINE_LOCK_NOTIFICATION(Hash, 0x0006, All);

	/// Proof and secret.
	DEFINE_LOCK_NOTIFICATION(Proof_Secret, 0x0007, Validator);

	/// Proof publication.
	DEFINE_LOCK_NOTIFICATION(Proof_Publication, 0x0008, All);

#undef DEFINE_LOCK_NOTIFICATION

	// endregion

	/// Notification of a hash lock mosaic.
	struct HashLockMosaicNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Lock_Mosaic_Notification;

	public:
		/// Creates a notification around \a mosaic.
		explicit HashLockMosaicNotification(model::Mosaic mosaic)
				: Notification(Notification_Type, sizeof(HashLockMosaicNotification))
				, Mosaic(mosaic)
		{}

	public:
		/// The mosaic.
		model::Mosaic Mosaic;
	};

	/// Base for lock duration notification.
	template<typename TDerivedNotification>
	struct BaseLockDurationNotification : public Notification {
	public:
		/// Creates a notification around \a duration.
		explicit BaseLockDurationNotification(BlockDuration duration)
				: Notification(TDerivedNotification::Notification_Type, sizeof(TDerivedNotification))
				, Duration(duration)
		{}

	public:
		/// The duration.
		BlockDuration Duration;
	};

	/// Notification of a hash lock duration
	struct HashLockDurationNotification : public BaseLockDurationNotification<HashLockDurationNotification> {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Lock_Hash_Duration_Notification;

	public:
		using BaseLockDurationNotification<HashLockDurationNotification>::BaseLockDurationNotification;
	};

	/// Notification of a secret lock duration
	struct SecretLockDurationNotification : public BaseLockDurationNotification<SecretLockDurationNotification> {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Lock_Secret_Duration_Notification;

	public:
		using BaseLockDurationNotification<SecretLockDurationNotification>::BaseLockDurationNotification;
	};

	/// Notification of a secret lock hash algorithm.
	struct SecretLockHashAlgorithmNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Lock_Hash_Algorithm_Notification;

	public:
		/// Creates secret lock hash algorithm notification around \a hashAlgorithm.
		SecretLockHashAlgorithmNotification(LockHashAlgorithm hashAlgorithm)
				: Notification(Notification_Type, sizeof(SecretLockHashAlgorithmNotification))
				, HashAlgorithm(hashAlgorithm)
		{}

	public:
		/// The hash algorithm.
		LockHashAlgorithm HashAlgorithm;
	};

	/// Base for lock transaction notification.
	template<typename TDerivedNotification>
	struct BaseLockNotification : public Notification {
	protected:
		/// Creates base lock notification around \a signer, \a mosaic and \a duration.
		BaseLockNotification(const Key& signer, const model::Mosaic& mosaic, BlockDuration duration)
				: Notification(TDerivedNotification::Notification_Type, sizeof(TDerivedNotification))
				, Signer(signer)
				, Mosaic(mosaic)
				, Duration(duration)
		{}

	public:
		/// The signer.
		const Key& Signer;

		/// The mosaic and amount.
		const model::Mosaic& Mosaic;

		/// The duration.
		BlockDuration Duration;
	};

	/// Notification of a hash lock.
	struct HashLockNotification : public BaseLockNotification<HashLockNotification> {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Lock_Hash_Notification;

	public:
		/// Creates hash lock notification around \a signer, \a mosaic, \a duration and \a hash.
		HashLockNotification(const Key& signer, const model::Mosaic& mosaic, BlockDuration duration, const Hash256& hash)
				: BaseLockNotification(signer, mosaic, duration)
				, Hash(hash)
		{}

	public:
		/// The hash.
		const Hash256& Hash;
	};

	/// Notification of a secret lock.
	struct SecretLockNotification : public BaseLockNotification<SecretLockNotification> {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Lock_Secret_Notification;

	public:
		/// Creates secret lock notification around \a signer, \a mosaic, \a duration, \a hashAlgorithm, \a secret and \a recipient.
		SecretLockNotification(
				const Key& signer,
				const model::Mosaic& mosaic,
				BlockDuration duration,
				LockHashAlgorithm hashAlgorithm,
				const Hash512& secret,
				const Address& recipient)
				: BaseLockNotification(signer, mosaic, duration)
				, HashAlgorithm(hashAlgorithm)
				, Secret(secret)
				, Recipient(recipient)
		{}

	public:
		/// The hash algorithm.
		LockHashAlgorithm HashAlgorithm;

		/// The secret.
		const Hash512& Secret;

		/// The recipient of the locked mosaic.
		const Address& Recipient;
	};

	/// Notification of a secret and its proof.
	struct ProofSecretNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Lock_Proof_Secret_Notification;

	public:
		/// Creates proof secret notification around \a hashAlgorithm, \a secret and \a proof.
		ProofSecretNotification(LockHashAlgorithm hashAlgorithm, const Hash512& secret, const RawBuffer& proof)
				: Notification(Notification_Type, sizeof(ProofSecretNotification))
				, HashAlgorithm(hashAlgorithm)
				, Secret(secret)
				, Proof(proof)
		{}

	public:
		/// The hash algorithm.
		LockHashAlgorithm HashAlgorithm;

		/// The secret.
		const Hash512& Secret;

		/// The proof.
		RawBuffer Proof;
	};

	/// Notification of a proof publication.
	struct ProofPublicationNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Lock_Proof_Publication_Notification;

	public:
		/// Creates proof publication notification around \a signer, \a hashAlgorithm and \a secret.
		ProofPublicationNotification(const Key& signer, LockHashAlgorithm hashAlgorithm, const Hash512& secret)
				: Notification(Notification_Type, sizeof(ProofPublicationNotification))
				, Signer(signer)
				, HashAlgorithm(hashAlgorithm)
				, Secret(secret)
		{}

	public:
		/// The signer.
		const Key& Signer;

		/// The hash algorithm.
		LockHashAlgorithm HashAlgorithm;

		/// The secret.
		const Hash512& Secret;
	};
}}
