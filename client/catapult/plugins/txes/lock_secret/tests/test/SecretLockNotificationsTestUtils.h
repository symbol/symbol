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
#include "tests/test/core/ResolverTestUtils.h"
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
			return model::SecretLockNotification(m_owner, m_mosaic, m_duration, m_hashAlgorithm, m_secret, m_recipient);
		}

		/// Prepares the builder using \a lockInfo.
		void prepare(const state::SecretLockInfo& lockInfo) {
			m_secret = lockInfo.Secret;
			m_recipient = test::UnresolveXor(lockInfo.RecipientAddress);
		}

	private:
		Address m_owner;
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
			test::FillWithRandomData(m_owner);
			test::FillWithRandomData(m_secret);
			test::FillWithRandomData(m_recipient);
		}

	public:
		/// Creates a notification.
		auto notification() const {
			return model::ProofPublicationNotification(m_owner, m_algorithm, m_secret, m_recipient);
		}

		/// Sets the notification \a height.
		void setHeight(Height height) {
			m_notificationHeight = height;
		}

		/// Sets the notification \a algorithm.
		void setAlgorithm(model::LockHashAlgorithm algorithm) {
			m_algorithm = algorithm;
		}

		/// Prepares the builder using \a lockInfo.
		void prepare(const state::SecretLockInfo& lockInfo) {
			m_secret = lockInfo.Secret;
			m_recipient = test::UnresolveXor(lockInfo.RecipientAddress);
		}

		/// Gets the notification height.
		auto notificationHeight() const {
			return m_notificationHeight;
		}

		/// Gets the notification hash.
		const auto& hash() const {
			return m_secret;
		}

		/// Gets the notification recipient.
		const auto& recipient() const {
			return m_recipient;
		}

	private:
		Height m_notificationHeight;
		model::LockHashAlgorithm m_algorithm;
		Address m_owner;
		Hash256 m_secret;
		UnresolvedAddress m_recipient;
	};
}}
