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
#include "catapult/crypto/KeyPair.h"
#include <string>

namespace catapult { namespace tools { namespace ssl {

	/// Certificate chain scenarios.
	enum class ScenarioId : uint16_t {
		/// Valid certificate chain.
		Valid_Certificate_Chain,

		/// Malformed node certificate signature.
		Malformed_Node_Certificate_Signature,

		/// Malformed CA certificate signature.
		Malformed_Ca_Certificate_Signature,

		/// Single-level self-signed certificate.
		Single_Self_Signed_Ca_Certificate,

		/// Two-level certificate chain with both certificates using the same key.
		Two_Level_Certificate_Chain_With_Same_Key,

		/// Valid three-level certificate chain.
		Three_Level_Certificate_Chain,

		/// Expired node certificate.
		Expired_Node_Certificate,

		/// Expired CA certificate.
		Expired_Ca_Certificate,

		/// Sentinel value.
		Max_Value
	};

	/// Generate certificates for \a scenarioId using \a caKeyPair inside \a certificateDirectory.
	void GenerateCertificateDirectory(crypto::KeyPair&& caKeyPair, const std::string& certificateDirectory, ScenarioId scenarioId);
}}}
