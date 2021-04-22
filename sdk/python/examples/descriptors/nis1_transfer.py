from symbolchain.core.facade.NisFacade import NisFacade


def descriptor_factory():
    sample_address = NisFacade.Address('TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C')

    return [
        # mosaics but no message
        {
            'type': 'transfer',
            'recipient_address': sample_address,
            'amount': 12345_000000
        },

        # message but no mosaics
        {
            'type': 'transfer',
            'recipient_address': sample_address,
            'message': 'You miss 100%% of the shots you don’t take'
        },

        # mosaics and message
        {
            'type': 'transfer',
            'recipient_address': sample_address,
            'amount': 12345_000000,
            'message': ' Wayne Gretzky'
        }
    ]
