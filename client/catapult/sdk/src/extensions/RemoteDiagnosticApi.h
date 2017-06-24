#pragma once
#include "plugins/services/hashcache/src/state/TimestampedHash.h"
#include "plugins/txes/namespace/src/model/MosaicInfo.h"
#include "plugins/txes/namespace/src/model/NamespaceInfo.h"
#include "catapult/model/AccountInfo.h"
#include "catapult/model/DiagnosticCounterValue.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace ionet { class PacketIo; } }

namespace catapult { namespace extensions {

	/// An api for retrieving diagnostic information from a remote node.
	class RemoteDiagnosticApi {
	public:
		virtual ~RemoteDiagnosticApi() {}

	public:
		/// Gets account infos for all accounts with addresses in \a addresses.
		virtual thread::future<model::AccountInfoRange> accountInfos(model::AddressRange&& addresses) const = 0;

		/// Returns all timestamped hashes in \a timestampedHashes that are unconfirmed, i.e. not in the hash cache.
		virtual thread::future<state::TimestampedHashRange> confirmTimestampedHashes(
				state::TimestampedHashRange&& timestampedHashes) const = 0;

		/// Returns diagnostic counter values.
		virtual thread::future<model::EntityRange<model::DiagnosticCounterValue>> diagnosticCounters() const = 0;

		/// Gets mosaic infos for all mosaic ids in \a mosaicIds.
		virtual thread::future<model::EntityRange<model::MosaicInfo>> mosaicInfos(model::EntityRange<MosaicId>&& mosaicIds) const = 0;

		/// Gets namespace infos for all namespace ids in \a namespaceIds.
		virtual thread::future<model::EntityRange<model::NamespaceInfo>>
				namespaceInfos(model::EntityRange<NamespaceId>&& namespaceIds) const = 0;
	};

	/// Creates a diagnostic api for interacting with a remote node with the specified io (\a pIo).
	std::unique_ptr<RemoteDiagnosticApi> CreateRemoteDiagnosticApi(const std::shared_ptr<ionet::PacketIo>& pIo);
}}
