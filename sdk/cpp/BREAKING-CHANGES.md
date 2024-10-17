# Breaking changes

## Changes between 2.0.0 and 3.x

### SDK

1. All transaction names follow python style more and require '_transaction_v1' suffix, so for example:

**REMOVED**: `'type': 'accountAddressRestriction'`

**NEW**: `'type': 'account_address_restriction_transaction_v1'`

2. Bit of syntactic sugar, that was valid earlier is no longer supported, example:

**REMOVED**:
```
    'type': 'transfer',
    'signer_public_key': 'TEST',
    'fee': 1000000,
    'deadline': 41998024783,
    'recipient_address': 'TD4PJKW5JP3CNHA47VDFIM25RCWTWRGT45HMPSA',
    'mosaics': [(0x2CF403E85507F39E, 1000000)]
```
**NEW**:
```py
    'type': 'transfer_transaction_v1',
    'signer_public_key': 'TEST',
    'fee': 1000000,
    'deadline': 41998024783,
    'recipient_address': 'TD4PJKW5JP3CNHA47VDFIM25RCWTWRGT45HMPSA',
    'mosaics': [
        {'mosaic_id': 0x2CF403E85507F39E, 'amount': 1000000}
    ]
```

3. Signature should always be attached via transaction factory, sample:
```py
signature = facade.sign_transaction(key_pair, transaction)
facade.transaction_factory.attach_signature(transaction, signature)
```

4. NEM transaction names are conforming to the schemas.
This only affects 'importance-transfer' that was available earlier, it is now available as 'account_key_link_transaction_v1'

**REMOVED**:
```py
	{
		'type': 'importance-transfer',
		'mode': 1,
		'remote_account_public_key': PublicKey('BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F')
	}
```
**NEW**:
```py
	{
		'type': 'account_key_link_transaction_v1',
		'link_action': 'link',
		'remote_public_key': 'BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F'
	}
```

### Generated code

1. `catbuffer` python package has been completely deprecated, low-level structures are available under SDK's `symbolchain.sc` (symbol)
and `symbolchain.nc` (nem) modules.

2. Structure of a module has been flattened, Dto suffix has been removed:

**REMOVED**
```
from symbol_catbuffer.NetworkTypeDto import NetworkTypeDto

...
NetworkTypeDto.PUBLIC_TEST
```

**NEW**
```py
from symbolchain.sc import NetworkType

NetworkType.TESTNET
```

3. (due to change in catbuffer schemas) EntityType has been split to TransactionType and BlockType

**REMOVED**
```
EntityTypeDto.VRF_KEY_LINK_TRANSACTION
```

**NEW**
```
TransactionType.VRF_KEY_LINK
```

4. Every transaction type has `TRANSACTION_VERSION` and `TRANSACTION_TYPE` properties
