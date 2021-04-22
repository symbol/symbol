from symbolchain.core.facade.SymFacade import SymFacade


def descriptor_factory():
    sample_address = SymFacade.Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y')
    sample_mosaic_id = 0x7EDCBA90FEDCBA90

    return [
        # allow incoming transactions only from address below
        {
            'type': 'accountAddressRestriction',
            'restriction_flags': 'address',
            'restriction_additions': [sample_address]
        },

        # block transactions outgoing to given address
        # note: block and allow restrictions are mutually exclusive, documentation
        # https://docs.symbolplatform.com/concepts/account-restriction.html#account-restriction
        {
            'type': 'accountAddressRestriction',
            'restriction_flags': 'address outgoing block',
            'restriction_additions': [sample_address]
        },

        {
            'type': 'accountMosaicRestriction',
            'restriction_flags': 'mosaic_id',
            'restriction_additions': [sample_mosaic_id]
        },

        # allow only specific transaction types
        {
            'type': 'accountOperationRestriction',
            'restriction_flags': 'outgoing',
            'restriction_additions': [
                'transfer_transaction',
                'account_key_link_transaction',
                'vrf_key_link_transaction',
                'voting_key_link_transaction',
                'node_key_link_transaction'
            ]
        }
    ]
