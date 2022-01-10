# Breaking changes

## Changes between 2.0.0 and 3.0.0

### SDK

1. All transaction names follow python style more, so for example:

**REMOVED**: `'type': 'accountAddressRestriction'`

**NEW**: `'type': 'account_address_restriction'`

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
    'type': 'transfer',
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

### Generated code

1. `catbuffer` python package has been completely deprecated, low-level structures are available under SDK's `symbolchain.sc` module

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
