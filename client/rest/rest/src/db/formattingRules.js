import catapult from 'catapult-sdk';

const ModelType = catapult.model.ModelType;
const convert = catapult.utils.convert;

export default {
	[ModelType.none]: value => value,
	[ModelType.binary]: value => convert.uint8ToHex(value.buffer),
	[ModelType.uint64]: value => [value.getLowBitsUnsigned(), value.getHighBits() >>> 0],
	[ModelType.objectId]: value => value.toHexString().toUpperCase(),
	[ModelType.string]: value => value.toString()
};
