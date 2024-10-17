from binascii import unhexlify

from symbolchain.sc import FinalizationEpoch, FinalizationPoint, FinalizedBlockHeader, Hash256, Height


def create_finalized_block_header():
	header = FinalizedBlockHeader()
	header.round.epoch = FinalizationEpoch(1002)
	header.round.point = FinalizationPoint(3)
	header.height = Height(720720)
	header.hash = Hash256(unhexlify('F8E133571827948BA7D72EE62714893EC3E5C0279B44C46F528D2AED9AA28B95'))
	return header


other = [
	{
		'schema_name': 'FinalizedBlockHeader',
		'descriptor': {
			'type': 'Finalized_block_header',
			'object': create_finalized_block_header()
		}
	},
]
