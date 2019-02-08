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
#include "SecretLockInfoCacheTestUtils.h"
#include "src/model/SecretLockNotifications.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	/// Secret lock notification builder.
	struct SecretLockNotificationBuilder {
	public:
		/// Creates secret lock notification builder.
		SecretLockNotificationBuilder() {
			FillWithRandomData({ reinterpret_cast<uint8_t*>(this), sizeof(SecretLockNotificationBuilder) });
		}

		/// Creates secret lock notification builder with \a lockHashAlgorithm.
		explicit SecretLockNotificationBuilder(model::LockHashAlgorithm lockHashAlgorithm) : SecretLockNotificationBuilder() {
			m_hashAlgorithm = lockHashAlgorithm;
		}

		/// Creates a notification.
		auto notification() {
			return model::SecretLockNotification(m_signer, m_mosaic, m_duration, m_hashAlgorithm, m_secret, m_recipient);
		}

		/// Sets notification hash to \a secret.
		void setHash(const Hash256& secret) {
			m_secret = secret;
		}

	private:
		Key m_signer;
		model::UnresolvedMosaic m_mosaic;
		BlockDuration m_duration;
		model::LockHashAlgorithm m_hashAlgorithm;
		Hash256 m_secret;
		UnresolvedAddress m_recipient;
	};

	/// Proof notification builder.
	struct ProofNotificationBuilder {
	public:
		/// Creates a proof notification builder.
		ProofNotificationBuilder()
				: ProofNotificationBuilder(test::GenerateRandomValue<Height>())
		{}

		/// Creates a proof notification builder around \a notificationHeight.
		explicit ProofNotificationBuilder(Height notificationHeight)
				: m_notificationHeight(notificationHeight)
				, m_algorithm(model::LockHashAlgorithm::Op_Sha3_256) {
				test::FillWithRandomData(m_signer);
				test::FillWithRandomData(m_hash);
		}

	public:
		/// Creates a notification.
		auto notification() const {
			return model::ProofPublicationNotification(m_signer, m_algorithm, m_hash);
		}

		/// Sets notification \a height.
		void setHeight(Height height) {
			m_notificationHeight = height;
		}

		/// Sets notification \a algorithm.
		void setAlgorithm(model::LockHashAlgorithm algorithm) {
			m_algorithm = algorithm;
		}

		/// Sets notification \a hash.
		void setHash(const Hash256& hash) {
			m_hash = hash;
		}

		/// Returns notification height.
		auto notificationHeight() const {
			return m_notificationHeight;
		}

		/// Returns notification hash.
		auto hash() const {
			return m_hash;
		}

	private:
		Height m_notificationHeight;
		model::LockHashAlgorithm m_algorithm;
		Key m_signer;
		Hash256 m_hash;
	};
}}
