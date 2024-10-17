from binascii import unhexlify

from symbolchain.sc import DetachedCosignature, Hash256, PublicKey, Signature


def create_detached_cosignature():
	cosignature = DetachedCosignature()
	cosignature.version = 0
	cosignature.signer_public_key = PublicKey('67FA12789F80766D329C7F687C5C5F889A82F5E9C3E7996AE4FFE48C34299DE7')
	cosignature.signature = Signature(
		'622C0CA6CC2EC0C48776FC24BF34FB7F4912B3718457A44D41A32DFCD3DBCEDD7D2AA65325ED925E86EDEAE6AB6CA54ED8B4C0DD090ED9DB3860D295DA9820ED'
	)
	cosignature.parent_hash = Hash256(unhexlify('61E0F8B9AB2FE3E008DCE1380FECDAF5BCFB1851247BF990771154177A0B7E78'))
	return cosignature


other = [
	{
		'schema_name': 'DetachedCosignature',
		'descriptor': {
			'type': 'detached_cosignature',
			'object': create_detached_cosignature()
		}
	},
]
