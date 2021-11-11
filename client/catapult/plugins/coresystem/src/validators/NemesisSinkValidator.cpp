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

#include "Validators.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::SignatureNotification;

	DECLARE_STATEFUL_VALIDATOR(NemesisSink, Notification)(
			Height additionalAllowedSignaturesHeight,
			const std::vector<Signature>& additionalAllowedSignatures) {
		return MAKE_STATEFUL_VALIDATOR(NemesisSink, ([additionalAllowedSignaturesHeight, additionalAllowedSignatures](
				const Notification& notification,
				const ValidatorContext& context) {
			auto isNemesisPublicKey = notification.SignerPublicKey == context.Network.NemesisSignerPublicKey;
			if (!isNemesisPublicKey || Height(1) == context.Height)
				return ValidationResult::Success;

			if (additionalAllowedSignaturesHeight != context.Height)
				return Failure_Core_Nemesis_Account_Signed_After_Nemesis_Block;

			auto isExplicitlyAllowed = additionalAllowedSignatures.cend() != std::find(
					additionalAllowedSignatures.cbegin(),
					additionalAllowedSignatures.cend(),
					notification.Signature);

			return isExplicitlyAllowed ? ValidationResult::Success : Failure_Core_Nemesis_Account_Signed_After_Nemesis_Block;
		}));
	}
}}
