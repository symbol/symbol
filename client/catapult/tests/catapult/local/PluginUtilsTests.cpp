#include "catapult/local/PluginUtils.h"
#include "catapult/validators/AggregateEntityValidator.h"
#include "tests/test/local/LocalTestUtils.h"

namespace catapult { namespace local {

#define TEST_CLASS PluginUtilsTests

	TEST(TEST_CLASS, CanCreateStatelessValidator) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManager();

		// Act:
		auto pEntityValidator = CreateStatelessValidator(*pPluginManager);

		// Assert:
		ASSERT_TRUE(!!pEntityValidator);

		// - notice that, as an implementation detail, the returned validator is an aggregate of one
		//   (the entity -> notification adapter)
		ASSERT_EQ(1u, pEntityValidator->names().size());
		EXPECT_EQ(pPluginManager->createStatelessValidator()->name(), pEntityValidator->names()[0]);
	}

	TEST(TEST_CLASS, CanCreateEntityObserver) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManager();

		// Act:
		auto pEntityObserver = CreateEntityObserver(*pPluginManager);

		// Assert:
		ASSERT_TRUE(!!pEntityObserver);
		EXPECT_EQ(pPluginManager->createObserver()->name(), pEntityObserver->name());
	}

	TEST(TEST_CLASS, CanCreateUndoEntityObserver) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManager();

		// Act:
		auto pEntityObserver = CreateUndoEntityObserver(*pPluginManager);

		// Assert:
		ASSERT_TRUE(!!pEntityObserver);
		EXPECT_EQ(pPluginManager->createObserver()->name(), pEntityObserver->name());
	}

	TEST(TEST_CLASS, CanCreatePermanentEntityObserver) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManager();

		// Act:
		auto pEntityObserver = CreatePermanentEntityObserver(*pPluginManager);

		// Assert:
		ASSERT_TRUE(!!pEntityObserver);
		EXPECT_EQ(pPluginManager->createPermanentObserver()->name(), pEntityObserver->name());
	}
}}
