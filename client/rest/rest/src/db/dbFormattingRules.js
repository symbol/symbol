const catapult = require('catapult-sdk');

const { ModelType, status } = catapult.model;
const { convert } = catapult.utils;

module.exports = {
	[ModelType.none]: value => value,
	[ModelType.binary]: value => convert.uint8ToHex(value.buffer),
	[ModelType.uint64]: value => [value.getLowBitsUnsigned(), value.getHighBits() >>> 0],
	[ModelType.objectId]: value => value.toHexString().toUpperCase(),
	[ModelType.string]: value => value.toString(),
	[ModelType.statusCode]: value => status.toString(value >>> 0)
};
