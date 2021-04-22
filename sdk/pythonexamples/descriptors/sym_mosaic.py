from symbolchain.core.facade.SymFacade import SymFacade
from symbolchain.core.sym.IdGenerator import generate_mosaic_id


def descriptor_factory():
    sample_address = SymFacade.Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y')

    return [
        {
            'type': 'mosaicDefinition',
            'duration': 1,
            'nonce': 123,
            'flags': 'transferable restrictable',
            'divisibility': 2
        },

        {
            'type': 'mosaicSupplyChange',
            'mosaic_id': generate_mosaic_id(sample_address, 123),
            'delta': 1000 * 100,  # assuming divisibility = 2
            'action': 'increase'
        }
    ]
