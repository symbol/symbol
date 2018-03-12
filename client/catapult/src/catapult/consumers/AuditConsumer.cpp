#include "AuditConsumer.h"
#include "ConsumerResultFactory.h"
#include "InputUtils.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/RawFile.h"
#include <boost/filesystem/path.hpp>

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
				io::Write(file, input.sourcePublicKey());

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
			boost::filesystem::path m_auditDirectory;
			mutable size_t m_id;
		};
	}

	disruptor::ConstDisruptorConsumer CreateAuditConsumer(const std::string& auditDirectory) {
		return AuditConsumer(auditDirectory);
	}
}}
