from binascii import unhexlify

from symbolchain.sc import DetachedCosignature, Hash256, PublicKey, Signature


def create_detached_cosignature():
	cosignature = DetachedCosignature()
	cosignature.version = 0
	cosignature.signer_public_key = PublicKey('67fa12789f80766d329c7f687c5c5f889a82f5e9c3e7996ae4ffe48c34299de7')
	cosignature.signature = Signature(
		'622c0ca6cc2ec0c48776fc24bf34fb7f4912b3718457a44d41a32dfcd3dbcedd7d2aa65325ed925e86edeae6ab6ca54ed8b4c0dd090ed9db3860d295da9820ed'
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
