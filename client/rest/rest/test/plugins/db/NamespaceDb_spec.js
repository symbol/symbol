import { expect } from 'chai';
import test from './utils/namespaceDbTestUtils';

describe('namespace db', () => {
	describe('namespace by id', () => {
		it('returns undefined for unknown namespace id', () => {
			// Arrange: create 5 namespaces and 5 inactive namespaces
			const owner = test.random.publicKey();
			const namespaces = test.db.createNamespaces(5, owner);

			// Assert:
			return test.db.runDbTest(
				namespaces,
				db => db.namespaceById([123, 456]),
				entity => { expect(entity).to.equal(undefined); });
		});

		it('returns root namespace for known level 0 namespace id', () => {
			// Arrange: create 5 namespaces and 5 inactive namespaces
			const owner = test.random.publicKey();
			const namespaces = test.db.createNamespaces(5, owner);

			// Assert:
			return test.db.runDbTest(
				namespaces,
				db => db.namespaceById([12303, 0]),
				entity => {
					expect(entity.namespace.depth).to.equal(1);
					expect(entity).to.deep.equal(namespaces[3]);
				});
		});

		it('returns level 1 child namespace for known level 1 namespace id', () => {
			// Arrange: create 5 namespaces and 5 inactive namespaces
			const owner = test.random.publicKey();
			const namespaces = test.db.createNamespaces(5, owner);

			// Assert:
			return test.db.runDbTest(
				namespaces,
				db => db.namespaceById([12301, 0]),
				entity => {
					expect(entity.namespace.depth).to.equal(2);
					expect(entity).to.deep.equal(namespaces[1]);
				});
		});

		it('returns level 2 child  namespace for known level 2 namespace id', () => {
			// Arrange: create 5 namespaces and 5 inactive namespaces
			const owner = test.random.publicKey();
			const namespaces = test.db.createNamespaces(5, owner);

			// Assert:
			return test.db.runDbTest(
				namespaces,
				db => db.namespaceById([12302, 0]),
				entity => {
					expect(entity.namespace.depth).to.equal(3);
					expect(entity).to.deep.equal(namespaces[2]);
				});
		});
	});

	describe('namespaces by owner', () => {
		function assertNamespaces(dbCallParams, allNamespaces, expectedNamespaces) {
			// Arrange:
			const dbEntities = allNamespaces;

			// Assert:
			return test.db.runDbTest(
				dbEntities,
				db => db.namespacesByOwner(...dbCallParams),
				namespaces => {
					// Assert:
					const ids = namespaces.map(namespace => namespace._id);
					const expectedIds = expectedNamespaces.map(namespace => namespace._id);
					expect(namespaces.length).to.equal(expectedNamespaces.length);
					expect(ids).to.deep.equal(expectedIds);
					expect(namespaces).to.deep.equal(expectedNamespaces);
				});
		}

		it('for account with no namespaces', () => {
			// Arrange: create 3 namespaces and 3 inactive namespaces
			const allNamespaces = test.db.createNamespaces(3, test.random.publicKey());
			const expectedNamespaces = [];

			// Assert:
			return assertNamespaces([test.random.publicKey()], allNamespaces, expectedNamespaces);
		});

		function createRandomNamespaces(startId, count) {
			const namespaces = [];
			for (let id = startId; id < startId + count; ++id)
				namespaces.push(test.db.createNamespace(id, test.random.publicKey(), 0, id, [1234], { start: 10 * id, end: 10 * (id + 1) }, false));

			return namespaces;
		}

		it('for account with namespaces', () => {
			// Arrange: create 10 namespaces, 10 inactive namespaces, and additional 5 namespace with random owner
			// note: reverse() is needed, because db api returns in descending order
			const owner = test.random.publicKey();
			const seedNamespaces = test.db.createNamespaces(10, owner);
			const activeNamespaces = seedNamespaces.slice(0, 10).reverse();
			const additionalNamespaces = createRandomNamespaces(20, 5);
			const allNamespaces = seedNamespaces.concat(additionalNamespaces);

			// Assert:
			return assertNamespaces([owner], allNamespaces, activeNamespaces);
		});

		it('query respects supplied document id', () => {
			// Arrange: create 10 namespaces and 10 inactive namespaces
			const owner = test.random.publicKey();
			const seedNamespaces = test.db.createNamespaces(10, owner);
			const activeNamespaces = seedNamespaces.slice(0, 10).reverse();
			const expectedNamespaces = activeNamespaces.slice(8);

			// Assert:
			return assertNamespaces([owner, activeNamespaces[7]._id.toString()], seedNamespaces, expectedNamespaces);
		});

		function assertPageSize(pageSize, expectedSize) {
			// Arrange: create 200 namespaces and 200 inactive namespaces
			const owner = test.random.publicKey();
			const seedNamespaces = test.db.createNamespaces(200, owner);
			const expectedNamespaces = seedNamespaces.slice(0, 200).reverse().slice(0, expectedSize);

			// Assert:
			expect(expectedSize).to.equal(expectedNamespaces.length);
			return assertNamespaces([owner, undefined, pageSize], seedNamespaces, expectedNamespaces);
		}

		// minimum and maximum values are set in CatapultDb ctor
		it('query respects page size', () => assertPageSize(12, 12));
		it('query ensures minimum page size', () => assertPageSize(5, 10));
		it('query ensures maximum page size', () => assertPageSize(150, 100));
	});
});
