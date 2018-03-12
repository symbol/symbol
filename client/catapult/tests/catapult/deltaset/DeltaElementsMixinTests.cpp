#include "catapult/deltaset/DeltaElementsMixin.h"
#include "tests/catapult/deltaset/test/BaseSetTestsInclude.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace deltaset {

#define TEST_CLASS DeltaElementsMixinTests

	namespace {
		using UnorderedMapMutableTraits = test::UnorderedMapTraits<MutableTypeTraits<test::MutableTestElement>>;
		using UnorderedMapMutablePointerTraits = test::UnorderedMapTraits<MutableTypeTraits<std::shared_ptr<test::MutableTestElement>>>;

		// need to wrap DeltaElementsMixin in cache-like object in order to use MAKE_DELTA_ELEMENTS_MIXIN_TESTS

		template<typename TTraits>
		struct DeltaProxy : public DeltaElementsMixin<typename TTraits::DeltaType> {
		private:
			using DeltaType = typename TTraits::DeltaType;

		public:
			explicit DeltaProxy(const std::shared_ptr<DeltaType>& pSetDelta)
					: DeltaElementsMixin<DeltaType>(*pSetDelta)
					, m_pSetDelta(pSetDelta)
			{}

		public:
			void insert(const typename TTraits::ElementType& element) {
				m_pSetDelta->insert(element);
			}

			void remove(unsigned int id) {
				m_pSetDelta->remove(std::make_pair(std::to_string(id), id));
			}

		private:
			std::shared_ptr<DeltaType> m_pSetDelta;
		};

		template<typename TTraits>
		struct CacheProxy {
		public:
			auto createDelta() {
				return std::make_unique<DeltaProxy<TTraits>>(m_set.rebase());
			}

			void commit() {
				m_set.commit();
			}

		private:
			typename TTraits::Type m_set;
		};

		struct BaseTraits {
			using IdType = unsigned int;

			static IdType MakeId(uint8_t id) {
				return id;
			}

			static IdType GetId(const test::MutableTestElement& element) {
				return element.Value;
			}
		};

		struct MutableTraits : public BaseTraits {
			using CacheType = CacheProxy<UnorderedMapMutableTraits>;
			using ValueType = test::MutableTestElement;

			static ValueType CreateWithId(uint8_t id) {
				ValueType element;
				element.Name = std::to_string(id);
				element.Value = id;
				return element;
			}
		};

		struct MutablePointerTraits : public BaseTraits {
			using CacheType = CacheProxy<UnorderedMapMutablePointerTraits>;
			using ValueType = std::shared_ptr<test::MutableTestElement>;

			static ValueType CreateWithId(uint8_t id) {
				auto pElement = std::make_shared<test::MutableTestElement>();
				*pElement = MutableTraits::CreateWithId(id);
				return pElement;
			}
		};
	}

	DEFINE_DELTA_ELEMENTS_MIXIN_TESTS(MutableTraits, _Mutable);
	DEFINE_DELTA_ELEMENTS_MIXIN_TESTS(MutablePointerTraits, _MutablePointer);
}}
