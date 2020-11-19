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

#include "AddressExtractionBlockChangeSubscriber.h"
#include "AddressExtractor.h"

namespace catapult { namespace addressextraction {

	namespace {
		class AddressExtractionBlockChangeSubscriber : public io::BlockChangeSubscriber {
		public:
			explicit AddressExtractionBlockChangeSubscriber(const AddressExtractor& extractor) : m_extractor(extractor)
			{}

		public:
			void notifyBlock(const model::BlockElement& blockElement) override {
				m_extractor.extract(const_cast<model::BlockElement&>(blockElement));
			}

			void notifyDropBlocksAfter(Height) override {
				// nothing to intercept
			}

		private:
			const AddressExtractor& m_extractor;
		};
	}

	std::unique_ptr<io::BlockChangeSubscriber> CreateAddressExtractionBlockChangeSubscriber(const AddressExtractor& extractor) {
		return std::make_unique<AddressExtractionBlockChangeSubscriber>(extractor);
	}
}}
