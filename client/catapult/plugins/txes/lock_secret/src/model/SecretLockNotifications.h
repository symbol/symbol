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

#pragma once
#include "src/model/LockHashAlgorithm.h"
#include "plugins/txes/lock_shared/src/model/LockNotifications.h"

namespace catapult { namespace model {

	// region lock secret notification types

/// Defines a lock secret notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_LOCKSECRET_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, LockSecret, DESCRIPTION, CODE)

	/// Lock secret duration.
	DEFINE_LOCKSECRET_NOTIFICATION(Secret_Duration, 0x0001, Validator);

	/// Lock hash algorithm.
	DEFINE_LOCKSECRET_NOTIFICATION(Hash_Algorithm, 0x0002, Validator);

	/// Lock secret.
	DEFINE_LOCKSECRET_NOTIFICATION(Secret, 0x0003, All);

	/// Proof and secret.
	DEFINE_LOCKSECRET_NOTIFICATION(Proof_Secret, 0x0004, Validator);

	/// Proof publication.
	DEFINE_LOCKSECRET_NOTIFICATION(Proof_Publication, 0x0005, All);

#undef DEFINE_LOCKSECRET_NOTIFICATION

	// endregion

	/// Notification of a secret lock duration
	struct SecretLockDurationNotification : public BaseLockDurationNotification<SecretLockDurationNotification> {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = LockSecret_Secret_Duration_Notification;

	public:
		using BaseLockDurationNotification<SecretLockDurationNotification>::BaseLockDurationNotification;
	};

	/// Notification of a secret lock hash algorithm.
	struct SecretLockHashAlgorithmNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = LockSecret_Hash_Algorithm_Notification;

	public:
		/// Creates secret lock hash algorithm notification around \a hashAlgorithm.
		SecretLockHashAlgorithmNotification(LockHashAlgorithm hashAlgorithm)
				: Notification(Notification_Type, sizeof(SecretLockHashAlgorithmNotification))
				, HashAlgorithm(hashAlgorithm)
		{}

	public:
		/// Hash algorithm.
		LockHashAlgorithm HashAlgorithm;
	};

	/// Notification of a secret lock.
	struct SecretLockNotification : public BaseLockNotification<SecretLockNotification> {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = LockSecret_Secret_Notification;

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
		/// Hash algorithm.
		LockHashAlgorithm HashAlgorithm;

		/// Secret.
		const Hash512& Secret;

		/// Recipient of the locked mosaic.
		Address Recipient;
	};

	/// Notification of a secret and its proof.
	struct ProofSecretNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = LockSecret_Proof_Secret_Notification;

	public:
		/// Creates proof secret notification around \a hashAlgorithm, \a secret and \a proof.
		ProofSecretNotification(LockHashAlgorithm hashAlgorithm, const Hash512& secret, const RawBuffer& proof)
				: Notification(Notification_Type, sizeof(ProofSecretNotification))
				, HashAlgorithm(hashAlgorithm)
				, Secret(secret)
				, Proof(proof)
		{}

	public:
		/// Hash algorithm.
		LockHashAlgorithm HashAlgorithm;

		/// Secret.
		const Hash512& Secret;

		/// Proof.
		RawBuffer Proof;
	};

	/// Notification of a proof publication.
	struct ProofPublicationNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = LockSecret_Proof_Publication_Notification;

	public:
		/// Creates proof publication notification around \a signer, \a hashAlgorithm and \a secret.
		ProofPublicationNotification(const Key& signer, LockHashAlgorithm hashAlgorithm, const Hash512& secret)
				: Notification(Notification_Type, sizeof(ProofPublicationNotification))
				, Signer(signer)
				, HashAlgorithm(hashAlgorithm)
				, Secret(secret)
		{}

	public:
		/// Signer.
		const Key& Signer;

		/// Hash algorithm.
		LockHashAlgorithm HashAlgorithm;

		/// Secret.
		const Hash512& Secret;
	};
}}
