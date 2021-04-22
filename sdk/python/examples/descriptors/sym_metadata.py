from symbolchain.core.facade.SymFacade import SymFacade


def descriptor_factory():
    sample_address = SymFacade.Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y')
    sample_namespace_id = 0xC01DFEE7FEEDDEAD
    sample_mosaic_id = 0x7EDCBA90FEDCBA90
    value1 = 'much coffe, such wow'
    value2 = 'Once upon a midnight dreary'
    value3 = 'while I pondered, weak and weary'

    return [
        {
            'type': 'accountMetadata',
            'target_address': sample_address,
            'scoped_metadata_key': 0xC0FFE,
            'value_size_delta': len(value1),
            'value': value1
        },

        {
            'type': 'mosaicMetadata',
            'target_mosaic_id': sample_mosaic_id,
            'target_address': sample_address,
            'scoped_metadata_key': 0xFACADE,
            'value_size_delta': len(value2),
            'value': value2
        },

        {
            'type': 'namespaceMetadata',
            'target_namespace_id': sample_namespace_id,
            'target_address': sample_address,
            'scoped_metadata_key': 0xC1CADA,
            'value_size_delta': len(value3),
            'value': value3
        }
    ]
