from symbolchain.core.facade.SymFacade import SymFacade


def descriptor_factory():
    sample_address = SymFacade.Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y')
    sample_namespace_id = 0xC01DFEE7FEEDDEAD
    sample_mosaic_id = 0x7EDCBA90FEDCBA90

    return [
        {
            'type': 'addressAlias',
            'namespace_id': sample_namespace_id,
            'address': sample_address,
            'alias_action': 'link'
        },

        {
            'type': 'mosaicAlias',
            'namespace_id': sample_namespace_id,
            'mosaic_id': sample_mosaic_id,
            'alias_action': 'link'
        }
    ]
