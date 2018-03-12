const catapult = require('catapult-sdk');

const { ModelType, status } = catapult.model;
const { convert } = catapult.utils;

module.exports = {
	[ModelType.none]: value => value,
	[ModelType.binary]: value => convert.uint8ToHex(value),
	[ModelType.uint64]: value => value,
	[ModelType.string]: value => value.toString(),
	[ModelType.statusCode]: status.toString
};
