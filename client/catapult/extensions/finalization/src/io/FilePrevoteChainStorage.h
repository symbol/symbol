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
#include "PrevoteChainStorage.h"

namespace catapult { namespace io {

	/// File prevote chain storage.
	class FilePrevoteChainStorage : public PrevoteChainStorage {
	public:
		/// Creates prevote chain storage around \a dataDirectory.
		explicit FilePrevoteChainStorage(const std::string& dataDirectory);

	public:
		bool contains(const model::FinalizationRound& round, const model::HeightHashPair& heightHashPair) const override;
		model::BlockRange load(const model::FinalizationRound& round, Height maxHeight) const override;
		void save(const BlockStorageView& blockStorageView, const PrevoteChainDescriptor& descriptor) override;
		void remove(const model::FinalizationRound& round) override;

	private:
		std::string m_dataDirectory;
	};
}}
