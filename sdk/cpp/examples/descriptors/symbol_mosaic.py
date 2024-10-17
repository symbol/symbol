from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.IdGenerator import generate_mosaic_id


def descriptor_factory():
	sample_address = SymbolFacade.Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y')

	return [
		{
			'type': 'mosaic_definition_transaction_v1',
			'duration': 1,
			'nonce': 123,
			'flags': 'transferable restrictable',
			'divisibility': 2
		},

		{
			'type': 'mosaic_supply_change_transaction_v1',
			'mosaic_id': generate_mosaic_id(sample_address, 123),
			'delta': 1000 * 100,  # assuming divisibility = 2
			'action': 'increase'
		}
	]
