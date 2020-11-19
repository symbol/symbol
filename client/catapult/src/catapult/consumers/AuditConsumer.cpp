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

#include "AuditConsumer.h"
#include "ConsumerResultFactory.h"
#include "InputUtils.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/RawFile.h"
#include <filesystem>

namespace catapult { namespace consumers {

	namespace {
		void Write(io::RawFile& file, const model::VerifiableEntity& entity) {
			file.write({ reinterpret_cast<const uint8_t*>(&entity), entity.Size});
		}

		class AuditConsumer {
		public:
			explicit AuditConsumer(const std::string& auditDirectory)
					: m_auditDirectory(auditDirectory)
					, m_id(0)
			{}

		public:
			ConsumerResult operator()(const disruptor::ConsumerInput& input) const {
				if (input.empty())
					return Abort(Failure_Consumer_Empty_Input);

				auto filename = (m_auditDirectory / std::to_string(++m_id)).generic_string();
				io::RawFile file(filename, io::OpenMode::Read_Write, io::LockMode::None);
				io::Write32(file, utils::to_underlying_type(input.source()));
				file.write(input.sourceIdentity().PublicKey);

				if (input.hasBlocks()) {
					for (const auto& element : input.blocks())
						Write(file, element.Block);
				} else {
					for (const auto& element : input.transactions())
						Write(file, element.Transaction);
				}

				return Continue();
			}

		private:
			std::filesystem::path m_auditDirectory;
			mutable size_t m_id;
		};
	}

	disruptor::ConstDisruptorConsumer CreateAuditConsumer(const std::string& auditDirectory) {
		return AuditConsumer(auditDirectory);
	}
}}
