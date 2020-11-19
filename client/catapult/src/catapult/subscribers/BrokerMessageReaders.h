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
#include "catapult/io/BufferInputStreamAdapter.h"
#include "catapult/io/FileQueue.h"
#include "catapult/utils/traits/Traits.h"

namespace catapult { namespace subscribers {

	// region Flusher

	namespace detail {
		template<typename TSubscriber, typename = void>
		struct Flusher {
			static void Flush(const TSubscriber&)
			{}
		};

		template<typename TSubscriber>
		struct Flusher<TSubscriber, utils::traits::is_type_expression_t<decltype(reinterpret_cast<TSubscriber*>(0)->flush())>> {
			static void Flush(TSubscriber& subscriber) {
				subscriber.flush();
			}
		};
	}

	// endregion

	/// Reads all messages from \a inputStream into \a subscriber using \a readNextMessage.
	template<typename TSubscriber, typename TMessageReader>
	void ReadAll(io::InputStream& inputStream, TSubscriber& subscriber, TMessageReader readNextMessage) {
		while (!inputStream.eof())
			readNextMessage(inputStream, subscriber);

		detail::Flusher<TSubscriber>::Flush(subscriber);
	}

	/// Reads all messages from \a reader into \a subscriber using \a readNextMessage.
	template<typename TSubscriber, typename TMessageReader>
	void ReadAll(io::FileQueueReader& reader, TSubscriber& subscriber, TMessageReader readNextMessage) {
		bool shouldContinue = true;
		while (shouldContinue) {
			shouldContinue = reader.tryReadNextMessage([&subscriber, readNextMessage](const auto& buffer) {
				io::BufferInputStreamAdapter<std::vector<uint8_t>> inputStream(buffer);
				ReadAll(inputStream, subscriber, readNextMessage);
			});
		}
	}

	/// Describes a message queue.
	struct MessageQueueDescriptor {
		/// Path of the message queue.
		std::string QueuePath;

		/// Name of index reader file.
		std::string IndexReaderFilename;

		/// Name of index writer file.
		std::string IndexWriterFilename;
	};

	/// Reads all messages from queue described by \a descriptor into \a subscriber using \a readNextMessage.
	template<typename TSubscriber, typename TMessageReader>
	void ReadAll(const MessageQueueDescriptor& descriptor, TSubscriber& subscriber, TMessageReader readNextMessage) {
		io::FileQueueReader reader(descriptor.QueuePath, descriptor.IndexReaderFilename, descriptor.IndexWriterFilename);

		auto numPendingMessages = reader.pending();
		if (0 == numPendingMessages)
			return;

		CATAPULT_LOG(debug) << "preparing to process " << numPendingMessages << " messages from " << descriptor.QueuePath;
		subscribers::ReadAll(reader, subscriber, readNextMessage);
	}
}}
