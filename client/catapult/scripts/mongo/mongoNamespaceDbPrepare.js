(function prepareNamespaceCollections() {
	db.createCollection('namespaces');
	db.namespaces.createIndex({ 'namespace.level0': 1, 'meta.index': 1, 'namespace.depth': 1 });
	db.namespaces.createIndex({ 'meta.active': -1, 'meta.index': 1, 'namespace.level0': 1, 'namespace.depth': 1 });
	db.namespaces.createIndex({ 'meta.active': -1, 'namespace.level1': 1, 'namespace.depth': 1 });
	db.namespaces.createIndex({ 'meta.active': -1, 'namespace.level2': 1, 'namespace.depth': 1 });
	db.namespaces.createIndex({ 'meta.active': -1, 'namespace.ownerAddress': 1 });
})();
