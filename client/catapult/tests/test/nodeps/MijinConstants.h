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
#include "catapult/types.h"
#include <cstring>

namespace catapult { namespace test {

	// private keys
	constexpr auto Mijin_Test_Nemesis_Private_Key = "B59D53B625CC746ECD478FA24F1ABC80B2031EB7DFCD009D5A74ADE615893175";
	constexpr const char* Mijin_Test_Private_Keys[] = {
		"934B1829665F7324362380E844CBEDA2C103AAEFD3A2C4645DC1715AC29E52E6",
		"83133E126ED380826B41B395FE64E3C989E9EE801FF198EDE46BF2E64A37DA79",
		"6ED36237A9ED61F6BB8A06C85A4C9A07AA15625A2876804073F680DB823CF83B",
		"B28691530189655A0CFE65025A91D22E68985B1015228DB8259ABAE33C0BFBD7",
		"98F08FBFA850C174EDDABE745FFD4180D20C07B4F91A6E73B39C7E1DC7CEA070",
		"F0E9FACD9BAF4B593D16D1E07130B4F430583E2F503367D556980624ED5E2AA1",
		"952C995AA82294C0C675AA24053716EB2B8058EC4A71C52F024376E8C947681A",
		"4385BBE4CE2F6101BDBB8660B2F1ABBC577D9E73F1DE4B47FD63659126B2AA98",
		"FACA8FB0278A7B13DBCF855C52442716CA6F734B152F97CBE0CE4EC216547B5C",
		"33CBFFC1CE4D671E2DE435ECE770B9B3637F18FB9E87BB6310E101952FDFA7CD",
		"C521E74AC0B33A34F8A4743FB67700CB2D78F5D25E0C05687DF025E630793864",
		"CDBA374FD14E9FEC51E832B193132BA73A812BE6901D9D0BABE17E8DAB3B2E7D",
		"3B35EF66D3AC8411A749D6457B6863F3FF74AD9F6399651EBA095F9164E8D04D",
		"8BD5243CDBAACF59C1970CB39C99D94F30417EA6A231ED8D6371E13660F06C35",
		"4C45D6CDFE089444D525B4CD97105020B0D2D4CDB74CFDE221B1B3F3903C2F06",
		"4E95CE574AE2DDBEE1A97B026BD3C68B42A9064F82297D563A8F3923006F9207",
		"29DF5D9B7C95FD6B8CFF04084FA69E4FAFA21496489BE88D918F064798FEBA5D",
		"CD74C4EF8E15991C7B37187293870A62FB151D44C8E186378DEC711A4CC07F09",
		"65179BAA70B3396C5ABE9B637B8713668B9CE4E50908F97C1EDFCEF98F3E5907",
		"6E2534CFCFD8F960C7C87928BF7673F6666593B9A53F95B9425B60B81ECA15B7",
		"FC273FE77A58D11AD08630FF06FCB4B49EC14CC01B4187104BA136998CF787E1",
		"3CA61068D040B9C88DF555C1DE8706486BB635845A66D3465A1A33C7FEE133D9"
	};

	// public keys
	constexpr auto Namespace_Rental_Fee_Sink_Public_Key = "3E82E1C1E4A75ADAA3CBA8C101C3CD31D9817A2EB966EB3B511FB2ED45B8E262";
	constexpr auto Mosaic_Rental_Fee_Sink_Public_Key = "53E140B5947F104CABC2D6FE8BAEDBC30EF9A0609C717D9613DE593EC2A266D3";

	/// Gets the nemesis importance for \a publicKey.
	inline Importance GetNemesisImportance(const Key& publicKey) {
		auto index = 0u;
		for (const auto* pRecipientPrivateKeyString : test::Mijin_Test_Private_Keys) {
			auto keyPair = crypto::KeyPair::FromString(pRecipientPrivateKeyString);
			if (keyPair.publicKey() == publicKey)
				break;

			++index;
		}

		// to simulate real harvesting mosaics, test nemesis block uses discrete importance seedings
		// only first 11 accounts have any harvesting power, two special accounts have importance 4X, nine other 1X
		if (index > 10)
			return Importance(0);

		return 4 == index || 10 == index ? Importance(4'000) : Importance(1'000);
	}
}}
