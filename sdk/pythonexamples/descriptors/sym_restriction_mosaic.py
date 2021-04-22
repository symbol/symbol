from symbolchain.core.facade.SymFacade import SymFacade


def descriptor_factory():
    sample_address = SymFacade.Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y')
    sample_mosaic_id = 0x7EDCBA90FEDCBA90

    return [
        {
            'type': 'mosaicGlobalRestriction',
            'mosaic_id': sample_mosaic_id,
            'reference_mosaic_id': 0,
            'restriction_key': 0x0A0D474E5089,
            'previous_restriction_value': 0,
            'new_restriction_value': 2,
            'previous_restriction_type': 0,
            'new_restriction_type': 'ge'
        },

        {
            'type': 'mosaicAddressRestriction',
            'mosaic_id': sample_mosaic_id,
            'restriction_key': 0x0A0D474E5089,
            'previous_restriction_value': 0,
            'new_restriction_value': 5,
            'target_address': sample_address
        }
    ]
