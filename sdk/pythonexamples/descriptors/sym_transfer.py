from symbolchain.core.facade.SymFacade import SymFacade


def descriptor_factory():
    sample_address = SymFacade.Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y').bytes
    sample_mosaic_id = 0x7EDCBA90FEDCBA90

    return [
        # mosaics but no message
        {
            'type': 'transfer',
            'recipient_address': sample_address,
            'mosaics': [
                (sample_mosaic_id, 12345_000000)
            ]
        },

        # message but no mosaics
        {
            'type': 'transfer',
            'recipient_address': sample_address,
            'message': 'Wayne Gretzky'
        },

        # mosaics and message
        {
            'type': 'transfer',
            'recipient_address': sample_address,
            'mosaics': [
                (sample_mosaic_id, 12345_000000)
            ],
            'message': 'You miss 100%% of the shots you don’t take'
        }
    ]
