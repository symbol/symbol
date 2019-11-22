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
#include "PacketSocket.h"

namespace catapult { namespace ionet {

	/// Packet socket decorator.
	/// \note This class is tested indirectly through its concrete usages.
	template<typename TWrapFactory>
	class PacketSocketDecorator : public PacketSocket {
	public:
		/// Creates a decorator around \a pSocket and \a wrapFactory.
		PacketSocketDecorator(const std::shared_ptr<PacketSocket>& pSocket, const TWrapFactory& wrapFactory)
				: m_pSocket(pSocket)
				, m_wrapFactory(wrapFactory)
				, m_pIo(m_wrapFactory.wrapIo(m_pSocket))
				, m_pReader(m_wrapFactory.wrapReader(m_pSocket))
		{}

	public:
		void write(const PacketPayload& payload, const WriteCallback& callback) override {
			m_pIo->write(payload, callback);
		}

		void read(const ReadCallback& callback) override {
			m_pIo->read(callback);
		}

		void readMultiple(const ReadCallback& callback) override {
			m_pReader->readMultiple(callback);
		}

	public:
		void stats(const StatsCallback& callback) override {
			m_pSocket->stats(callback);
		}

		void waitForData(const WaitForDataCallback& callback) override {
			m_pSocket->waitForData(callback);
		}

		void close() override{
			m_pSocket->close();
		}

		std::shared_ptr<PacketIo> buffered() override {
			return m_wrapFactory.wrapIo(m_pSocket->buffered());
		}

	private:
		std::shared_ptr<PacketSocket> m_pSocket;
		TWrapFactory m_wrapFactory;
		std::shared_ptr<PacketIo> m_pIo;
		std::shared_ptr<BatchPacketReader> m_pReader;
	};
}}
