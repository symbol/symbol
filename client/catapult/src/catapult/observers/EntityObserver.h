#pragma once
#include "ObserverContext.h"
#include "catapult/model/VerifiableEntity.h"
#include "catapult/model/WeakEntityInfo.h"

namespace catapult { namespace observers {

	/// A weakly typed entity observer.
	/// \note This intended to be used only for execution-only situations (e.g. block loading and rollback).
	class EntityObserver {
	public:
		virtual ~EntityObserver() {}

	public:
		/// Gets the observer name.
		virtual const std::string& name() const = 0;

		/// Notifies the observer with an \a entityInfo to process and an observer \a context.
		virtual void notify(const model::WeakEntityInfo& entityInfo, const ObserverContext& context) const = 0;
	};
}}
