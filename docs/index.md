# Introduction to Symbol / Core Concepts

## Comparisons

Most users will compare Symbol to one of the two major blockchain protocols: Bitcoin (a non-turing complete platform) or Ethereum (a turing complete platform). It's important to understand that blockchains are *tools*, and as a developer you want to pick the right tool for the right job.

However, high-level comparisons can always be helpful for both researchers and developers. Here's how Symbol compares to both.

### Bitcoin

Bitcoin is focused on being *the best money*.

### Ethereum

Ethereum is focused on being *a global compute platform*. It's main innovation is the 'EVM' - the Ethereum Virtual Machine. [...]

However, a turing complete platform introduces unintended security risks and backdoors [...]

### Symbol

Symbol is designed to sit between Bitcoin and Ethereum - it focuses on allowing you to *plug-in* a decentralized ledger into your existing applications, and allows you to build on-chain applications with predictable behaviors and outcomes.

## Accounts

Accounts are addresses on Symbol that can hold mosaics, metadata and namespaces. Every user or node on the network is defined by one (or more) accounts.

### Restrictions
### Mnemonics
### Wallets
#### Deterministic vs Hierarchical Deterministic

### Tutorial: Creating an Account

Create a random account for a network by creating a `SymbolFacade` of the desired network. For most instances, it should be created around the name of a well-known network: "mainnet" or "testnet". All examples in this guide will use `SymbolNetwork("testnet")`

```python
def create_random_account(facade):
	# create a signing key pair that will be associated with an account
	key_pair = facade.KeyPair(PrivateKey.random())

	# convert the public key to a network-dependent address (unique account identifier)
	address = facade.network.public_key_to_address(key_pair.public_key)

	# output account public and private details
	print(f'    address: {address}')
	print(f' public key: {key_pair.public_key}')
	print(f'private key: {key_pair.private_key}')
```

example output:
```
    address: TCEVUET3MJE73F2VG6G3LRWKZN4A3DLX4WJ5XBA
 public key: D1CBF707D990A8C08C3EF68EFECF25B684934C16D9C8BE8B32D34DC511F13070
private key: 91597A3C1FD648D630FEEB339351C168D0581F46F07FA13277F26D5EE0D40283
```

Alternatively, a seed phrase can be used (or randomly generated) and used to derive accounts.

```python
def create_random_bip32_account(facade):
	# create a random Bip39 seed phrase (mnemonic)
	bip32 = Bip32()
	mnemonic = bip32.random()

	# derive a root Bip32 node from the mnemonic and a password 'correcthorsebatterystaple'
	root_node = bip32.from_mnemonic(mnemonic, 'correcthorsebatterystaple')

	# derive a child Bip32 node from the root Bip32 node for the account at index 0
	child_node = root_node.derive_path(facade.bip32_path(0))

	# convert the Bip32 node to a signing key pair
	key_pair = facade.bip32_node_to_key_pair(child_node)

	# convert the public key to a network-dependent address (unique account identifier)
	address = facade.network.public_key_to_address(key_pair.public_key)

	# output account public and private details
	print(f'   mnemonic: {mnemonic}')
	print(f'    address: {address}')
	print(f' public key: {key_pair.public_key}')
	print(f'private key: {key_pair.private_key}')
```

example output:
```
   mnemonic: east actual egg series spot express addict always human swallow decrease turn surround direct place burst million curious dish divorce net nephew allow height
    address: TBDSOVXFLHZWDLGSEBEE5Z5SLD2DP7P2VDXYB7Y
 public key: E2CCAD62EEBB5826042776796D26D66611EE84411C3CDF0CA5E0B4CC2FCFBE4D
private key: 984D4E4EC6AB5C772876135D88DF40F13B7B5880324A6D7F19E16DB292F8C443
```

### Tutorial: Key Derivation and Verification

:warning: what examples here?

### Tutorial: Create an Account and Fund via Faucet

```python
async def create_account_with_tokens_from_faucet(facade, amount=150, private_key=None):
	# create a key pair that will be used to send transactions
	# when the PrivateKey is known, pass the raw private key bytes or hex encoded string to the PrivateKey(...) constructor instead
	key_pair = facade.KeyPair(PrivateKey.random()) if private_key is None else facade.KeyPair(private_key)
	address = facade.network.public_key_to_address(key_pair.public_key)
	print(f'new account created with address: {address}')

	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP POST request to faucet endpoint
		request = {
			'recipient': str(address),
			'amount': amount,
			'selectedMosaics': ['3A8416DB2D53B6C8']  # XYM mosaic id on testnet
		}
		async with session.post(f'{SYMBOL_TOOLS_ENDPOINT}/claims', json=request) as response:
			# wait for the (JSON) response
			response_json = await response.json()

			# extract the funding transaction hash and wait for it to be confirmed
			transaction_hash = Hash256(response_json['txHash'])
			await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='funding transaction')

	return key_pair
```

### Tutorial: Querying the Balance of an Account

Account state can be easily queried using `/accounts/<account-id>` identifier.

**Query by address:**
```sh
curl https://${SYMBOL_API_NODE}:3001/accounts/TA4RYHMNHCFRCT2PCWOCJMWVAQ3ZCJDOTF2SGBI
```

**Query by public key:**
```sh
curl https://${SYMBOL_API_NODE}:3001/accounts/23AC0770A1060241604A8E60A47166E3E5B4034D4EE321DBE19B342E85B21544
```

Getting actual balance in a generic fashion is a bit more complicated.

First network currency id needs to be retrieved.
```python
async def get_network_currency():
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint
		async with session.get(f'{SYMBOL_API_ENDPOINT}/network/properties') as response:
			# wait for the (JSON) response
			properties = await response.json()

			# exctract currency mosaic id
			mosaic_id = int(properties['chain']['currencyMosaicId'].replace('\'', ''), 0)
			print(f'currency mosaic id: {mosaic_id}')
			return mosaic_id
```

Next to get currency mosaic divisibility, mosaic properties needs to be retrieved.
```python
async def get_mosaic_properties(mosaic_id):
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint
		async with session.get(f'{SYMBOL_API_ENDPOINT}/mosaics/{mosaic_id}') as response:
			# wait for the (JSON) response
			return await response.json()
```

Finally account state can be queried and all pieces can glued together. `account.mosaics` needs to be searched for currency. Additionally amount is formatted using obtained mosaic divisibility.

```python
async def get_account_state():
	account_identifier = 'TA4RYHMNHCFRCT2PCWOCJMWVAQ3ZCJDOTF2SGBI'  # Address or public key

	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint
		async with session.get(f'{SYMBOL_API_ENDPOINT}/accounts/{account_identifier}') as response:
			# wait for the (JSON) response
			return await response.json()
```
```python
async def get_account_balance():
	network_currency_id = await get_network_currency()
	network_currency_id_formatted = f'{network_currency_id:08X}'

	currency_mosaic = await get_mosaic_properties(network_currency_id_formatted)
	divisibility = currency_mosaic['mosaic']['divisibility']

	account_state = await get_account_state()

	# search for currency inside account mosaics
	account_currency = next(mosaic for mosaic in account_state['account']['mosaics'] if network_currency_id == int(mosaic['id'], 16))
	amount = int(account_currency['amount'])
	account_balance = {
		'balance': {
			'id': account_currency['id'],
			'amount': amount,
			'formatted_amount': f'{amount // 10**divisibility}.{(amount % 10**divisibility):0{divisibility}}'
		}
	}

	print(account_balance)
	return account_balance
```


### Tutorial: Querying State of an Account (Current & Historical)

```python
async def get_account_state():
	account_identifier = 'TA4RYHMNHCFRCT2PCWOCJMWVAQ3ZCJDOTF2SGBI'  # Address or public key

	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint
		async with session.get(f'{SYMBOL_API_ENDPOINT}/accounts/{account_identifier}') as response:
			# wait for the (JSON) response
			return await response.json()
```

TODO: @jaguar, what else should go here, do we want to do anything re historical?

### Tutorial: Adding or Modifying (Account) Metadata

Account can have assigned metadata. Metadata is assigned to address and either can be assigned via _own_ account or via some other account.
However, to avoid spamming account metadata by third parties, `account metadata transaction` needs to always be wrapped in an aggregate (therefore it automatically requires account owner's cosignature).

Note, account metadata, as well as other kinds of metadata transactions, are designed to attach data that _might_ change in future, good examples are things like home webpage URI, avatar, etc.

There might be better ways to store (or simply encode) the data that is not expected to change.

**Assigning metadata to own account:**

```python
async def create_account_metadata_new(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# metadata transaction needs to be wrapped in aggregate transaction

	value = 'https://twitter.com/NCOSIGIMCITYNREmalformed'.encode('utf8')

	embedded_transactions = [
		facade.transaction_factory.create_embedded({
			'type': 'account_metadata_transaction',
			'signer_public_key': signer_key_pair.public_key,

			# the key consists of a tuple (signer, target_address, scoped_metadata_key)
			#  - if signer is different than target address, the target account will need to cosign the transaction
			#  - scoped_metadata_key can be any 64-bit value picked by metadata creator
			'target_address': facade.network.public_key_to_address(signer_key_pair.public_key),
			'scoped_metadata_key': 0x72657474697774,

			'value_size_delta': len(value),  # when creating _new_ value this needs to be equal to value size
			'value': value
		})
	]
	# create the transaction
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'aggregate_complete_transaction',
		'transactions_hash': facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'account metadata (new) transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='account metadata (new) transaction')
```

**Modifying existing metadata:**

When changing/updating existing data, passed value needs to be "xor" result of old and new values, there's a helper for that
!py symbolchain.symbol.Metadata.metadata_update_value
!js symbol.metadata.metadataUpdateValue

```python
async def create_account_metadata_modify(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# metadata transaction needs to be wrapped in aggregate transaction

	# to update existing metadata, new value needs to be 'xored' with previous value.
	old_value = 'https://twitter.com/NCOSIGIMCITYNREmalformed'.encode('utf8')
	new_value = 'https://twitter.com/0x6861746366574'.encode('utf8')
	update_value = metadata_update_value(old_value, new_value)

	embedded_transactions = [
		facade.transaction_factory.create_embedded({
			'type': 'account_metadata_transaction',
			'signer_public_key': signer_key_pair.public_key,

			# the key consists of a tuple (signer, target_address, scoped_metadata_key),
			# when updating all values must match previously used values
			'target_address': facade.network.public_key_to_address(signer_key_pair.public_key),
			'scoped_metadata_key': 0x72657474697774,

			'value_size_delta': len(new_value) - len(old_value),  # change in size, negative because the value will be shrunk
			'value': update_value
		})
	]
	# create the transaction
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'aggregate_complete_transaction',
		'transactions_hash': facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'account metadata (modify) transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='account metadata (modify) transaction')
```

### Tutorial: Adding, Modifying or Removing a Cosignatory

In Symbol an account can be turned into multisig account using `multisig account modification` transaction. Modification requires cosignatures of all involved parties, so multisig account modification transactions is only allowed as an transaction within aggregate transaction.

To actually cosign transactions, private keys of cosignatories are needed. In example below, the code has access to all private keys, of course, in reality every cosignatory will need to cosign on their own.

Moreover, example below uses _simpler_ aggregate complete, when if there are different cosignatories, it would be much more convenient to use aggregate bonded transaction (TODO: explain why it's easier to cosign bonded tx)

Transaction preparation can be split into three phases:
 1. preparations of multisig account and cosignatories,
 2. transaction preparation - important part here is to sign aggregate prior to adding cosignatures,
 3. adding cosignatures - this part might look bit weird, that is because it needs to convert some of SDK types into low-level catbuffer types from `symbolchain.sc` module.

```python
async def create_multisig_account_modification_new_account(facade, signer_key_pair):
	# pylint: disable=too-many-locals
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create cosignatory key pairs, where each cosignatory will be required to cosign initial modification
	# (they are insecurely deterministically generated for the benefit of related tests)
	cosignatory_key_pairs = [facade.KeyPair(PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, i]))) for i in range(3)]
	cosignatory_addresses = [facade.network.public_key_to_address(key_pair.public_key) for key_pair in cosignatory_key_pairs]

	# multisig account modification transaction needs to be wrapped in aggregate transaction

	embedded_transactions = [
		facade.transaction_factory.create_embedded({
			'type': 'multisig_account_modification_transaction',
			'signer_public_key': signer_key_pair.public_key,

			'min_approval_delta': 2,  # number of signatures required to make any transaction
			'min_removal_delta': 2,  # number of signatures needed to remove a cosignatory from multisig
			'address_additions': cosignatory_addresses
		})
	]

	# create the transaction, notice that signer account that will be turned into multisig is a signer of transaction
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'aggregate_complete_transaction',
		'transactions_hash': facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'multisig account (create) transaction hash {transaction_hash}')

	# cosign transaction by all partners (this is dependent on the hash and consequently the main signature)
	for cosignatory_key_pair in cosignatory_key_pairs:
		cosignature = facade.cosign_transaction(cosignatory_key_pair, transaction)
		transaction.cosignatures.append(cosignature)

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='multisig account (create) transaction')
```

After this transaction 2-of-3 cosignatories are required to make any transaction, same goes for removal from multisig, due to `min_removal_delta`.

Following example shows how two of cosignatories can swap third one for some other one. Additionally altering amount of cosignatories required for removal (`min_removal_delta`) - example is bit artificial, cause in effect single cosignatory can remove all others, which makes multisig account quite insecure.

```python
async def create_multisig_account_modification_modify_account(facade, signer_key_pair):
	# pylint: disable=too-many-locals
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	cosignatory_key_pairs = [facade.KeyPair(PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, i]))) for i in range(4)]
	cosignatory_addresses = [facade.network.public_key_to_address(key_pair.public_key) for key_pair in cosignatory_key_pairs]

	# multisig account modification transaction needs to be wrapped in aggregate transaction

	embedded_transactions = [
		# create a transfer from the multisig account to the primary cosignatory to cover the transaction fee
		facade.transaction_factory.create_embedded({
			'type': 'transfer_transaction',
			'signer_public_key': signer_key_pair.public_key,

			'recipient_address': cosignatory_addresses[0],
			'mosaics': [
				{'mosaic_id': generate_mosaic_alias_id('symbol.xym'), 'amount': 5_000000}
			]
		}),

		facade.transaction_factory.create_embedded({
			'type': 'multisig_account_modification_transaction',
			'signer_public_key': signer_key_pair.public_key,  # sender of modification transaction is multisig account

			# don't change number of cosignature needed for transactions
			'min_approval_delta': 0,
			# decrease number of signatures needed to remove a cosignatory from multisig (optional)
			'min_removal_delta': -1,
			'address_additions': [cosignatory_addresses[3]],
			'address_deletions': [cosignatory_addresses[1]]
		})
	]

	# create the transaction, notice that account that will be turned into multisig is a signer of transaction
	transaction = facade.transaction_factory.create({
		'signer_public_key': cosignatory_key_pairs[0].public_key,  # signer of the aggregate is one of the two cosignatories
		'deadline': network_time.timestamp,

		'type': 'aggregate_complete_transaction',
		'transactions_hash': facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(cosignatory_key_pairs[0], transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'multisig account (modify) transaction hash {transaction_hash}')

	# cosign transaction by all partners (this is dependent on the hash and consequently the main signature)
	for cosignatory_key_pair in [cosignatory_key_pairs[2], cosignatory_key_pairs[3]]:
		cosignature = facade.cosign_transaction(cosignatory_key_pair, transaction)
		transaction.cosignatures.append(cosignature)

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='multisig account (modify) transaction')
```

Note, that the aggregate transaction is signed by `cosignatory[0]` key pair, but "signer" (or rather _sender_) of the modification transaction is `multisig_key_pair`.

Cosignature of a cosignatory that is added to multisig is ALWAYS required, independent of current settings of `min_approval` or `min_removal`. Reason for this is pretty straight-forward, newly added account must "agree" to actually become cosignatory.

### Tutorial: Vanity Generation and You

Addresses are produced from account's public key, exact format is described inside [technical reference](https://github.com/symbol/symbol-technicalref). It's only important to note, that address is a result of applying [one-way function](https://en.wikipedia.org/wiki/One-way_function) on public key.
Raw symbol addresses are 24-bytes long with first byte indicating network type; for display/presentation purposes raw addresses are passed through [base32](https://en.wikipedia.org/wiki/Base32) encoding.

Combining those informations, usually addresses will look random, like so: `NDR6EW2WBHJQDYMNGFX2UBZHMMZC5PGL2YCZOQQ`

There are two _vanity generators_ within symbol, which you can use to search for addresses that contain certain substring.
In general every vanity generator works by searching for random secret keys, then producing public keys, then producing addresses and matching against user supplied string.

The two vanity generators are:
 * first one that is available in [symbol/product/tools/vanity](https://github.com/symbol/product),
 * second comes with catapult client, it's called `catapult.tools.addressgen`.

Note, currently both vanity generators provide BIP-39 mnemonic which can be used in wallet apps.

1. **product/tools/vanity** - this generator is a python script, that is supposed to be called as a module:
   ```shell
   $ cd symbol/product/tools/vanity
   $ python -m vanity --blockchain symbol --network testnet --patterns HELLO --format pretty
   address (testnet): TAHELLOCN5XFWRIAWKSPPYMATZHGXTJEI52NGBQ
          public key: 77643BA9D1C7B3D05B8C6BDDDAB17BE5BADEF17E94746628ED321DE4E56D4967
         private key: 9A2FC95ACB385EEC8F7AA6DDC0BC45A36A32F904038EC988E3418858994164CB
            mnemonic: twice despair october tenant swamp second harvest lens mom violin catch response naive stomach divorce captain humble kite income ranch help bacon asthma enhance
   ```
   It can search for multiple strings at once:
   ```shell
   $ python -m vanity --blockchain symbol --network mainnet --patterns JPG,TXT,DOC --format pretty
   address (mainnet): NDDOCGCLCXT5UYOCR62KTTF5LBOGYJIQG4T7TPA
          public key: 20D84535171838BEDC663A59ABFB131668BC3226AF44DCCAC627CFC3835F5D97
         private key: 7760F90593A88A079BD650F0A2982AC8F3F08B960D47466D1D2E922D28D9B7A8
         mnemonic: vibrant february claim pact shine flash outdoor cube come menu train kick elbow vague illness lawsuit win episode motor squeeze ginger winter scrub razor

   address (mainnet): NDJPGDCSYTDKFZDBPEWT5VHJTHYH6BE27CNLOGI
          public key: A4202C89A878CA6988916AD5C12D51BDBD1CFAAF8A547E95581E1F6C6C70E667
         private key: AF30757226DBDE393AFECE48949DB17AAC77DFDCDE3D1D53F8DAB66C72D22C30
            mnemonic: tired father have permit cup tonight symptom keen churn box alien ginger one slow despair action clip stick demise segment magic steel minute harvest

   address (mainnet): NDTXTYRR4CQD2WOUDC4U37BSW4DOXK245OWDNEY
          public key: 2909CBC4031A4F6633220EF3B5E64861046807F458D580FB352981563287C03F
         private key: 4996E13DEDAB2449DE504F32C61AF0DCD5121B672B74EB440CF7FB20097FDDD2
         mnemonic: witness just change dentist congress find hurry surround smile lucky chest idea valid kick actual scale brother blind float broken twin reflect poet once
   ```
2. **catapult.tools.addressgen**, example invocation:
   ```shell
   $ ./bin/catapult.tools.addressgen --network mainnet --input HELLO
   ...
        address (mainnet): NANFMSZRRIZDHELLO77SWVRFY53KGO3EWLOEN2Q
          address decoded: 681A564B318A3233916B77FF2B5625C776A33B64B2DC46EA
               public key: C20829A3EE22A9943B8A0AB0893D699CF2D6A07716A8D1789501249C64D88B2E
              private key: 93-please-dont-use-this-its-just-for-demonstration-purposes-6410
                 mnemonic: deer grid tonight gym royal wear topple amazing message item lend tortoise    bounce carpet toward spatial camera xxx xxx xxx xxx xxx xxx coconut
   ```
   While searching for 5 chars might finish in few minutes, search time increases expotentially with every    character. Searching for strings containing characters outside of base32 alphabet (i.e. `0`, `1`, `8`,    `9`) will never finish.

   Due to how base32 encoding works, the only availible prefixes in mainnet are `NA, NB, NC, ND`, to    search starting at the beginning of an address prefix input with a caret sign `^`:
   ```shell
   $ ./bin/catapult.tools.addressgen --network mainnet --input ^NAHELL
        address (mainnet): NAHELLACJKBYBQGQ7ZGLOOYDFWKE2ZSWB3A3HDQ
          address decoded: 680E45AC024A8380C0D0FE4CB73B032D944D66560EC1B38E
               public key: B939FF4BA0F86812A6315E0D5DA179A0FD4384CE11F291B8A02E6BE46F8EFA7A
              private key: 42-please-dont-use-this-its-just-for-demonstration-purposes-2736
                 mnemonic: man crouch imitate about carry choice idea spend nose thank merit isolate equal    raw direct spray spread xxx xxx xxx xxx xxx xxx three
   ```

## Namespaces

Namespaces are human-readable names for an account or a mosaic.

Similar to ENS for Ethereum, Bonafida for SOL or Namebase for Handshake, namespaces are intended to map human-readable names like "hatchet.axe" to machine-readable identifiers such as addresses, metadata, hashes, and more.

Symbol network has 3 levels of namespaces defined via `maxNamespaceDepth` in `config-network.properties`. Top level namespace is called _root_ namespace.

### Duration

Namespaces have limited duration expressed in number of blocks, defined  by root-level namespace. There are two settings defining duration settings `minNamespaceDuration`, `maxNamespaceDuration`.

TODO: explain renewal and `namespaceGracePeriodDuration`

### Fees

Similar to actual domains, namespaces require fees and renewal.
Currently cost of root namespace is 2 xym per block of duration, multiplied by dynamic fee multiplier (default = 100).
Cost of child namespace is defined by `childNamespaceRentalFee` setting in `config-network.properties`
which gets multiplied by dynamic fee multiplier (default = 100).

In 'mainnet' value of `childNamespaceRentalFee` is 10.

### Tutorial: Creating or Extending a Namespace

Namespaces besides having maximal duration, also have minimal duration, in 'mainnet' that is 30d (2880 * 30 blocks).

```python
async def create_namespace_registration_root(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create the transaction
	namespace_name = f'who_{str(signer_address).lower()}'
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'namespace_registration_transaction',
		'registration_type': 'root',  # 'root' indicates a root namespace is being created
		'duration': 86400,  # number of blocks the root namespace will be active; approximately 30 (86400 / 2880) days
		'name': namespace_name  # name of the root namespace
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'namespace (root) registration transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='namespace (root) registration transaction')
```

Registration of root namespace generates `BalanceTransferReceipt` with type `NamespaceRentalFee`.

Child namespace:
```python
async def create_namespace_registration_child(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create the transaction
	root_namespace_name = f'who_{str(signer_address).lower()}'
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'namespace_registration_transaction',
		'registration_type': 'child',  # 'child' indicates a namespace will be attach to some existing root namespace
		'parent_id': generate_namespace_id(root_namespace_name),  # this points to root namespace
		'name': 'killed'  # name of the child namespace
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'namespace (child) registration transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='namespace (child) registration transaction')
```

Registration of child namespace also generates `BalanceTransferReceipt` with type `NamespaceRentalFee`.

### Tutorial: Adding, Modifying or Querying (Namespace) Metadata

Namespaces - like accounts - can have assigned metadata (compare with [Tutorial: Adding or Modifying Metadata](#Tutorial:-Adding-or-Modifying-Metadata)).

Example below assumes signer is also owner of the namespace.

```python
async def create_namespace_metadata_new(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# metadata transaction needs to be wrapped in aggregate transaction

	root_namespace_name = f'who_{str(signer_address).lower()}'
	value = 'Laura Palmer'.encode('utf8')

	embedded_transactions = [
		facade.transaction_factory.create_embedded({
			'type': 'namespace_metadata_transaction',
			'signer_public_key': signer_key_pair.public_key,

			# the key consists of a tuple (signer, target_address, target_namespace_id, scoped_metadata_key)
			#  - if signer is different than target address, the target account will need to cosign the transaction
			#  - target address must be namespace owner
			#  - namespace with target_namespace_id must exist
			#  - scoped_metadata_key can be any 64-bit value picked by metadata creator
			'target_address': facade.network.public_key_to_address(signer_key_pair.public_key),
			'target_namespace_id': generate_namespace_id('killed', generate_namespace_id(root_namespace_name)),
			'scoped_metadata_key': int.from_bytes(b'name', byteorder='little'),

			'value_size_delta': len(value),
			'value': value
		})
	]
	# create the transaction
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'aggregate_complete_transaction',
		'transactions_hash': facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'namespace metadata (new) transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='namespace metadata (new) transaction')
```

Modify the above metadata.
```python
async def create_namespace_metadata_modify(facade, signer_key_pair):  # pylint: disable=too-many-locals
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# metadata transaction needs to be wrapped in aggregate transaction

	root_namespace_name = f'who_{str(signer_address).lower()}'
	old_value = 'Laura Palmer'.encode('utf8')
	new_value = 'Catherine Martell'.encode('utf8')
	update_value = metadata_update_value(old_value, new_value)

	embedded_transactions = [
		facade.transaction_factory.create_embedded({
			'type': 'namespace_metadata_transaction',
			'signer_public_key': signer_key_pair.public_key,

			# the key consists of a tuple (signer, target_address, target_namespace_id, scoped_metadata_key)
			# when updating all values must match previously used values
			'target_address': facade.network.public_key_to_address(signer_key_pair.public_key),
			'target_namespace_id': generate_namespace_id('killed', generate_namespace_id(root_namespace_name)),
			'scoped_metadata_key': int.from_bytes(b'name', byteorder='little'),

			'value_size_delta': len(new_value) - len(old_value),
			'value': update_value
		})
	]
	# create the transaction
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'aggregate_complete_transaction',
		'transactions_hash': facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'namespace metadata (modify) transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='namespace metadata (modify) transaction')
```

Query namespace metadata:
```sh
curl https://${SYMBOL_API_NODE}:3001/metadata?targetId=D51E852A906C2DFA&scopedMetadataKey=00000000656D616E`
```

## Mosaics

In Symbol, all tokens (including the base layer currency, $XYM) are referred to as mosaics. You can think of mosaics as similar to an ERC-20 token from Ethereum, or a colored coin from Bitcoin.

Rather than having multiple *types* of tokens, Symbol instead employs a ruleset to define how tokens can be traded, brought or sold.

The current rulesets that can be defined for mosaics are: supply, transfer, and revoke.

Supply defines the total supply of a mosaic - that is, your token's 'cap'. It must be within a range of 0&ndash;8'999'999'999'000'000 atomic units. The limit is defined by `[insert reason here]`, and can be modified by changing `maxMosaicAtomicUnits` inside `config-network.settings`, in case of existing network this would result in hard fork and some more changes in the client would be needed inside https://github.com/symbol/symbol/blob/dev/client/catapult/plugins/txes/mosaic/src/validators/MosaicSupplyChangeAllowedValidator.cpp. The supply ruleset also has an additional flag, allowing for "modifiable" or "fixed" - modifiable means the total supply can be altered by the creator, whereas fixed means the total supply is defined at creation and can not be altered.

Transfer specifies who you can transfer your mosaic to - it can be open (and thus the creator and subsequent owner can transfer it to any account on the network), or you can specify a whitelist of addresses (inclusive) or a blacklist of addresses (exclusive).

Revoke allows the creator of a mosaic to recall the supply from holders **at any time**.

### Tutorial: Creating a Mosaic

Creating mosaic is either 2 or 3 step process:
 1. create mosaic definition
 2. create mosaic supply - which actually mints units
 3. (optional) create and link namespace id to mosaic

```python
async def create_mosaic_definition_new(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'mosaic_definition_transaction',
		'duration': 0,  # number of blocks the mosaic will be active; 0 indicates it will never expire
		'divisibility': 2,  # number of supported decimal places

		# nonce is used as a locally unique identifier for mosaics with a common owner
		# mosaic id is derived from the owner's address and the nonce
		'nonce': 123,

		# set of restrictions to apply to the mosaic
		# - 'transferable' indicates the mosaic can be freely transfered among any account that can own the mosaic
		# - 'restrictable' indicates that the owner can restrict the accounts that can own the mosaic
		'flags': 'transferable restrictable'
	})

	# transaction.id field is mosaic id and it is filled automatically after calling transaction_factory.create()

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'mosaic definition transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='mosaic definition transaction')
```

**Create a supply:**

Mosaic above has id `0x1788BA84888894EB`, following transaction will increase it's supply. Supply needs to be specified in atomic unis. Mosaic has divisibility set to 2, so to create 123 mosaics `12300` needs to be specified as number of units.

```python
async def create_mosaic_supply(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'mosaic_supply_change_transaction',
		'mosaic_id': generate_mosaic_id(signer_address, 123),

		# action can either be 'increase' or 'decrease',
		# if mosaic does not have 'mutable supply' flag, owner can issue supply change transactions only if owns full supply
		'action': 'increase',

		# delta is always unsigned number, it's specified in atomic units, created mosaic has divisibility set to 2 decimal places,
		# so following delta will result in 100 units
		'delta': 100_00
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'mosaic supply transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='mosaic supply transaction')
```

TODO: should there be namespace link here as well?

### Tutorial: Working with Rules (restrictions?)

There are two kind of mosaic restrictions:
 * global
 * address-based

Global restrictions allow to define global rules, that determine if account is able to send or receive given mosaic.

Mosaic from [creating a mosaic](#Tutorial:-Creating-a-Mosaic) example has id `0x1788BA84888894EB`.

```python
async def create_global_mosaic_restriction_new(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'mosaic_global_restriction_transaction',
		'mosaic_id': generate_mosaic_id(signer_address, 123),

		# restriction might use some other mosaic restriction rules, that mosaic doesn't even have to belong to current owner
		'reference_mosaic_id': 0,
		'restriction_key': 0xC0FFE,
		'previous_restriction_type': 0,  # this is newly created restriction so there was no previous type
		'previous_restriction_value': 0,

		# 'ge' means greater or equal, possible operators are: 'eq', 'ne', 'lt', 'le', 'gt', 'ge'
		'new_restriction_type': 'ge',
		'new_restriction_value': 1
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'global mosaic restriction (new) transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='global mosaic restriction (new) transaction')
```

After this transaction in order to be able to send mosaic to anyone,
owner first need to set mosaic address restrictions **including own account**.

It's called restriction, but technically this is addresss-based mosaic-level metadata, that is accessed by global restriction rule.

```python
async def create_address_mosaic_restriction_1(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'mosaic_address_restriction_transaction',
		'mosaic_id': generate_mosaic_id(signer_address, 123),

		'restriction_key': 0xC0FFE,
		'previous_restriction_value': 0xFFFFFFFF_FFFFFFFF,
		'new_restriction_value': 10,
		'target_address': facade.network.public_key_to_address(signer_key_pair.public_key)
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'address mosaic restriction (new:1) transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='address mosaic restriction (new:1) transaction')
```
```python
async def create_address_mosaic_restriction_2(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'mosaic_address_restriction_transaction',
		'mosaic_id': generate_mosaic_id(signer_address, 123),

		'restriction_key': 0xC0FFE,
		'previous_restriction_value': 0xFFFFFFFF_FFFFFFFF,
		'new_restriction_value': 1,
		'target_address': SymbolFacade.Address('TBOBBYKOYQBWK3HSX7NQVJ5JFPE22352AVDXXAA')
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'address mosaic restriction (new:2) transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='address mosaic restriction (new:2) transaction')
```
```python
async def create_address_mosaic_restriction_3(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'mosaic_address_restriction_transaction',
		'mosaic_id': generate_mosaic_id(signer_address, 123),

		'restriction_key': 0xC0FFE,
		'previous_restriction_value': 0xFFFFFFFF_FFFFFFFF,
		'new_restriction_value': 2,
		'target_address': SymbolFacade.Address('TALICECI35BNIJQA5CNUKI2DY3SXNEHPZJSOVAA')
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'address mosaic restriction (new:3) transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='address mosaic restriction (new:3) transaction')
```

Notice, that **TBOBBY** account has value set to 1, while **TALICE** set to 2, this will be used later.

Owner can send some `0x1788BA84888894EB` mosaic to two other accounts, both can transfer it as well.

```python
async def create_global_mosaic_restriction_modify(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'mosaic_global_restriction_transaction',
		'mosaic_id': generate_mosaic_id(signer_address, 123),

		'reference_mosaic_id': 0,
		'restriction_key': 0xC0FFE,
		'previous_restriction_type': 'ge',  # must match old restriction type
		'previous_restriction_value': 1,  # must match old restriction value

		'new_restriction_type': 'ge',
		'new_restriction_value': 2
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'global mosaic restriction (modify) transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='global mosaic restriction (modify) transaction')
```

After this transaction **TALICE** can send and receive transaction, but **TBOBBY** cannot.
**TBOBBY** will get `Failure_RestrictionMosaic_Account_Unauthorized` as a transaction status when trying to send the mosaic.

### Tutorial: Adding, Modifying or Querying (Mosaic) Metadata

Similar to account metadata, mosaic can have assigned metadata.
The key used to access metadata is a pair: `(mosaic id, scoped key)`.
Target address needs to be set to mosaic owner address.

In a similar way to account metadata, mosaic metadata always require to be wrapped within an aggregate.

In future there some scoped keys might be standarized to be used across different issuers.

**Simple mosaic metadata assignment:**

```python
async def create_mosaic_metadata_new(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# metadata transaction needs to be wrapped in aggregate transaction

	value = unhexlify(
		'89504e470d0a1a0a0000000d49484452000000010000000108000000003a7e9b55'
		'0000000a49444154185763f80f00010101005a4d6ff10000000049454e44ae426082')

	embedded_transactions = [
		facade.transaction_factory.create_embedded({
			'type': 'mosaic_metadata_transaction',
			'signer_public_key': signer_key_pair.public_key,

			# the key consists of a tuple (signer, target_address, target_mosaic_id, scoped_metadata_key)
			#  - if signer is different than target address, the target account will need to cosign the transaction
			#  - target address must be mosaic owner
			#  - mosaic with target_mosaic_id must exist
			#  - scoped_metadata_key can be any 64-bit value picked by metadata creator
			'target_address': signer_address,
			'target_mosaic_id': generate_mosaic_id(signer_address, 123),
			'scoped_metadata_key': int.from_bytes(b'avatar', byteorder='little'),  # this can be any 64-bit value picked by creator

			'value_size_delta': len(value),  # when creating _new_ value this needs to be equal to value size
			'value': value
		})
	]
	# create the transaction
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'aggregate_complete_transaction',
		'transactions_hash': facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'mosaic metadata (new) transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='mosaic metadata (new) transaction')
```

**Attaching metadata to mosaic via third party:**

```python
async def create_mosaic_metadata_cosigned_1(facade, signer_key_pair):
	# pylint: disable=too-many-locals
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	authority_semi_deterministic_key = PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, 0]))
	authority_key_pair = await create_account_with_tokens_from_faucet(facade, 100, authority_semi_deterministic_key)

	# set new high score for an account

	value = (440).to_bytes(4, byteorder='little')

	embedded_transactions = [
		facade.transaction_factory.create_embedded({
			'type': 'mosaic_metadata_transaction',
			'signer_public_key': authority_key_pair.public_key,

			# the key consists of a tuple (signer, target_address, target_mosaic_id, scoped_metadata_key)
			#  - if signer is different than target address, the target account will need to cosign the transaction
			#  - target address must be mosaic owner
			#  - mosaic with target_mosaic_id must exist
			#  - scoped_metadata_key can be any 64-bit value picked by metadata creator
			'target_mosaic_id': generate_mosaic_id(signer_address, 123),
			'scoped_metadata_key': int.from_bytes(b'rating', byteorder='little'),  # this can be any 64-bit value picked by creator
			'target_address': signer_address,

			'value_size_delta': len(value),  # when creating _new_ value this needs to be equal to value size
			'value': value
		})
	]
	# create the transaction
	transaction = facade.transaction_factory.create({
		'signer_public_key': authority_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'aggregate_complete_transaction',
		'transactions_hash': facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(authority_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'mosaic metadata (cosigned 1) transaction hash {transaction_hash}')

	# cosign transaction by all partners (this is dependent on the hash and consequently the main signature)
	for cosignatory_key_pair in [signer_key_pair]:
		cosignature = facade.cosign_transaction(cosignatory_key_pair, transaction)
		transaction.cosignatures.append(cosignature)

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='mosaic metadata (cosigned 1) transaction')
```

**Modify metadata to mosaic via third party:**

```python
async def create_mosaic_metadata_cosigned_2(facade, signer_key_pair):
	# pylint: disable=too-many-locals
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	authority_semi_deterministic_key = PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, 0]))
	authority_key_pair = await create_account_with_tokens_from_faucet(facade, 100, authority_semi_deterministic_key)

	# update high score for an account

	old_value = (440).to_bytes(4, byteorder='little')
	new_value = (9001).to_bytes(4, byteorder='little')
	update_value = metadata_update_value(old_value, new_value)

	embedded_transactions = [
		facade.transaction_factory.create_embedded({
			'type': 'mosaic_metadata_transaction',
			'signer_public_key': authority_key_pair.public_key,

			# the key consists of a tuple (signer, target_address, target_mosaic_id, scoped_metadata_key)
			# when updating all values must match previously used values
			'target_mosaic_id': generate_mosaic_id(signer_address, 123),
			'scoped_metadata_key': int.from_bytes(b'rating', byteorder='little'),  # this can be any 64-bit value picked by creator
			'target_address': signer_address,

			# this should be difference between sizes, but this example does not change the size, so delta = 0
			'value_size_delta': 0,
			'value': update_value
		})
	]
	# create the transaction
	transaction = facade.transaction_factory.create({
		'signer_public_key': authority_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'aggregate_complete_transaction',
		'transactions_hash': facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(authority_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'mosaic metadata (cosigned 2) transaction hash {transaction_hash}')

	# cosign transaction by all partners (this is dependent on the hash and consequently the main signature)
	for cosignatory_key_pair in [signer_key_pair]:
		cosignature = facade.cosign_transaction(cosignatory_key_pair, transaction)
		transaction.cosignatures.append(cosignature)

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='mosaic metadata (cosigned 2) transaction')
```

**Quering mosaic state:**

TODO: not sure if we should list all possible ways to query metadata here, especially if the API is subject to change...

```sh
curl https://${SYMBOL_API_NODE}:3001/metadata?targetId=1788BA84888894EB&scopedMetadataKey=0000000074736574
```

```python
async def get_mosaic_metadata(facade, signer_key_pair):
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)

	mosaic_id = generate_mosaic_id(signer_address, 123)
	scoped_metadata_key = int.from_bytes(b'rating', byteorder='little')

	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint
		params = {
			'targetId': f'{mosaic_id:016X}',
			'scopedMetadataKey': f'{scoped_metadata_key:016X}'
		}
		async with session.get(f'{SYMBOL_API_ENDPOINT}/metadata', params=params) as response:
			# wait for the (JSON) response
			response_json = await response.json()

			print(json.dumps(response_json, indent=4))
			return response_json
```

### Tutorial: Performing an Atomic Swap

Atomic swaps within Symbol network are trivial, thanks to aggregate transactions.

Example below is using complete aggregate transaction, meaning both parties sign transaction before announcing it to the network.
Alternative would be creating a lock and bonded aggregate transaction, so that the other party could cosign simply by announcing cosignature to the network.

Cross-chain swaps can be found in [advanced topics](#Advanced-Topics) section.

**TA4RYH** wants to send 200 xym to **TALICE** in exchange for 1 piece of mosaic `0x64B6D476EC60C150`.

```python
async def create_mosaic_atomic_swap(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a second signing key pair that will be used as the swap partner
	partner_key_pair = await create_account_with_tokens_from_faucet(facade)

	# Alice (signer) owns some amount of custom mosaic (with divisibility=2)
	# Bob (partner) wants to exchange 20 xym for a single piece of Alice's custom mosaic
	# there will be two transfers within an aggregate
	embedded_transactions = [
		facade.transaction_factory.create_embedded({
			'type': 'transfer_transaction',
			'signer_public_key': partner_key_pair.public_key,

			'recipient_address': facade.network.public_key_to_address(signer_key_pair.public_key),
			'mosaics': [
				{'mosaic_id': generate_mosaic_alias_id('symbol.xym'), 'amount': 20_000000}
			]
		}),

		facade.transaction_factory.create_embedded({
			'type': 'transfer_transaction',
			'signer_public_key': signer_key_pair.public_key,

			'recipient_address': facade.network.public_key_to_address(partner_key_pair.public_key),
			'mosaics': [
				{'mosaic_id': generate_mosaic_id(signer_address, 123), 'amount': 100}
			]
		})
	]

	# Alice will be signer of aggregate itself, that also means he won't have to attach his cosignature
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'aggregate_complete_transaction',
		'transactions_hash': facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})

	# Bob needs to cosign the transaction because the swap will only be confirmed if both the sender and the partner agree to it

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'mosaic swap transaction hash {transaction_hash}')

	# cosign transaction by all partners (this is dependent on the hash and consequently the main signature)
	for cosignatory_key_pair in [partner_key_pair]:
		cosignature = facade.cosign_transaction(cosignatory_key_pair, transaction)
		transaction.cosignatures.append(cosignature)

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='mosaic swap transaction')
```

### Tutorial: Querying State of a Mosaic (Current, Historical, Namespace)

Query mosaic state:
```sh
curl https://${SYMBOL_API_NODE}:3001/mosaics/1788BA84888894EB
```

:warning: we don't have historical support currently

## Transactions

### Simple
### Aggregate
### Multisignature
### Atomic

TODO: isn't it the same / similar to 3.mosaics "Tutorial: Performing an Atomic Swap"

### Tutorial: Querying Unconfirmed Transactions

```python
async def wait_for_transaction_status(transaction_hash, desired_status, **kwargs):
	transaction_description = kwargs.get('transaction_description', 'transaction')
	async with ClientSession(raise_for_status=False) as session:
		for _ in range(600):
			# query the status of the transaction
			async with session.get(f'{SYMBOL_API_ENDPOINT}/transactionStatus/{transaction_hash}') as response:
				# wait for the (JSON) response
				response_json = await response.json()

				# check if the transaction has transitioned
				if 200 == response.status:
					status = response_json['group']
					print(f'{transaction_description} {transaction_hash} has status "{status}"')
					if desired_status == status:
						explorer_url = SYMBOL_EXPLORER_TRANSACTION_URL_PATTERN.format(transaction_hash)
						print(f'{transaction_description} has transitioned to {desired_status}: {explorer_url}')
						return

					if 'failed' == status:
						print(f'{transaction_description} failed validation: {response_json["code"]}')
						break
				else:
					print(f'{transaction_description} {transaction_hash} has unknown status')

			# if not, wait 20s before trying again
			time.sleep(20)

		# fail if the transaction didn't transition to the desired status after 10m
		raise RuntimeError(f'{transaction_description} {transaction_hash} did not transition to {desired_status} in alloted time period')
```

### Tutorial: Working with Encrypted Messages

```python
def decrypt_utf8_message(key_pair, public_key, encrypted_payload):
	message_encoder = MessageEncoder(key_pair)
	(is_decode_success, plain_message) = message_encoder.try_decode(public_key, encrypted_payload)
	if is_decode_success:
		print(f'decrypted message: {plain_message.decode("utf8")}')
	else:
		print(f'unable to decrypt message: {hexlify(encrypted_payload)}')
```

```python
async def create_transfer_with_encrypted_message(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic recipient (it insecurely deterministically generated for the benefit of related tests)
	recipient_key_pair = facade.KeyPair(PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, 0])))
	recipient_address = facade.network.public_key_to_address(recipient_key_pair.public_key)
	print(f'recipient: {recipient_address}')

	# encrypt a message using a signer's private key and recipient's public key
	message_encoder = MessageEncoder(signer_key_pair)
	encrypted_payload = message_encoder.encode(recipient_key_pair.public_key, 'this is a secret message'.encode('utf8'))
	print(f'encrypted message: {hexlify(encrypted_payload)}')

	# the same encoder can be used to decode a message
	decrypt_utf8_message(signer_key_pair, recipient_key_pair.public_key, encrypted_payload)

	# alternatively, an encoder around the recipient private key and signer public key can decode the message too
	decrypt_utf8_message(recipient_key_pair, signer_key_pair.public_key, encrypted_payload)

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'transfer_transaction',
		'recipient_address': recipient_address,
		'mosaics': [
			{'mosaic_id': generate_mosaic_alias_id('symbol.xym'), 'amount': 7_000000},  # send 7 of XYM to recipient
		],
		'message': encrypted_payload
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'transfer transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='transfer transaction')
```

### Tutorial: Persistent Delegated Messages

```python
async def create_harvesting_delegation_message(facade, signer_key_pair):
	# pylint: disable=too-many-locals
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic remote account (insecurely deterministically generated for the benefit of related tests)
	# this account will sign blocks on behalf of the (funded) signing account
	remote_key_pair = facade.KeyPair(PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, 0])))
	print(f'remote public key: {remote_key_pair.public_key}')

	# create a deterministic VRF account (insecurely deterministically generated for the benefit of related tests)
	# this account will inject randomness into blocks harvested by the (funded) signing account
	vrf_key_pair = facade.KeyPair(PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, 1])))
	print(f'VRF public key: {vrf_key_pair.public_key}')

	# create a deterministic node public key (insecurely deterministically generated for the benefit of related tests)
	# this account will be asked to host delegated harvesting of the (funded) signing account
	node_public_key = facade.KeyPair(PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, 2]))).public_key
	print(f'node public key: {node_public_key}')

	# create a harvesting delecation request message using the signer's private key and the remote node's public key
	# the signer's remote and VRF private keys will be shared with the node
	# in order to deactivate, these should be regenerated
	message_encoder = MessageEncoder(signer_key_pair)
	harvest_request_payload = message_encoder.encode_persistent_harvesting_delegation(node_public_key, remote_key_pair, vrf_key_pair)
	print(f'harvest request message: {hexlify(harvest_request_payload)}')

	# the same encoder can be used to decode a message
	decrypt_utf8_message(signer_key_pair, node_public_key, harvest_request_payload)

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'transfer_transaction',
		'recipient_address': facade.network.public_key_to_address(node_public_key),
		'message': harvest_request_payload
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'transfer transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='transfer transaction')
```

### Tutorial: Querying or Modifying Deadlines

:warning: not sure what this means?

### Tutorial: Signing Simple, Aggregate or Multisignature Transactions
*Manually, Automatically, Verification*

### Tutorial: Partial (Bonded) Transactions

```python
async def create_hash_lock(facade, signer_key_pair, bonded_transaction_hash):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'hash_lock_transaction',
		'mosaic': {'mosaic_id': generate_mosaic_alias_id('symbol.xym'), 'amount': 10_000000},

		'duration': 100,
		'hash': bonded_transaction_hash
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'hash lock transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='hash lock transaction')
```
```python
async def create_multisig_account_modification_new_account_bonded(facade, signer_key_pair):
	# pylint: disable=too-many-locals
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create cosignatory key pairs, where each cosignatory will be required to cosign initial modification
	# (they are insecurely deterministically generated for the benefit of related tests)
	cosignatory_key_pairs = [facade.KeyPair(PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, i]))) for i in range(3)]
	cosignatory_addresses = [facade.network.public_key_to_address(key_pair.public_key) for key_pair in cosignatory_key_pairs]

	embedded_transactions = [
		facade.transaction_factory.create_embedded({
			'type': 'multisig_account_modification_transaction',
			'signer_public_key': signer_key_pair.public_key,

			'min_approval_delta': 2,  # number of signatures required to make any transaction
			'min_removal_delta': 2,  # number of signatures needed to remove a cosignatory from multisig
			'address_additions': cosignatory_addresses
		})
	]

	# create the transaction, notice that signer account that will be turned into multisig is a signer of transaction
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'aggregate_bonded_transaction',
		'transactions_hash': facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'multisig account modification bonded (new) {transaction_hash}')

	# print the signed transaction, including its signature
	print(transaction)

	# create a hash lock transaction to allow the network to collect cosignaatures for the aggregate
	await create_hash_lock(facade, signer_key_pair, transaction_hash)

	# submit the partial (bonded) transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions/partial', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions/partial: {response_json}')

		# wait for the partial transaction to be cached by the network
		await wait_for_transaction_status(transaction_hash, 'partial', transaction_description='bonded aggregate transaction')

		# submit the (detached) cosignatures to the network
		for cosignatory_key_pair in cosignatory_key_pairs:
			cosignature = facade.cosign_transaction(cosignatory_key_pair, transaction, True)
			cosignature_json_payload = json.dumps({
				'version': str(cosignature.version),
				'signerPublicKey': str(cosignature.signer_public_key),
				'signature': str(cosignature.signature),
				'parentHash': str(cosignature.parent_hash)
			})
			print(cosignature_json_payload)

			# initiate a HTTP PUT request to a Symbol REST endpoint
			async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions/cosignature', json=json.loads(cosignature_json_payload)) as response:
				response_json = await response.json()
				print(f'/transactions/cosignature: {response_json}')

	# wait for the partial transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='bonded aggregate transaction')
```

## Nodes (API, Peer, Dual, Voting)

### Validators
alice

### Tutorial: Setting up a Voting Node
bob

### Tutorial: Setting up a Voting Node...for the Paranoid
charlie

### Tutorial: Key Rotation

To provide some forward security, voters are required to register their voting keys via voting link (below).
Voting link - besides voting public key - contains start and end epoch, when set of voting keys will be valid.

Voting keys are generated upfront and needs to be rotated.

```python
def create_voting_key_file(facade):
	# create a voting key pair
	voting_key_pair = facade.KeyPair(PrivateKey.random())

	# create a file generator
	generator = VotingKeysGenerator(voting_key_pair)

	# generate voting key file for epochs 10-150
	buffer = generator.generate(10, 150)

	# store to file
	# note: additional care should be taken to create file with proper permissions
	with open('private_key_tree1.dat', 'wb') as output_file:
		output_file.write(buffer)

	# show voting key public key
	print(f'voting key public key {voting_key_pair.public_key}')
```

### Harvesters
### Tutorial: Delegating your Stake

#### Harvesting Links

```python
async def create_account_key_link(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic remote account (insecurely deterministically generated for the benefit of related tests)
	# this account will sign blocks on behalf of the (funded) signing account
	remote_key_pair = facade.KeyPair(PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, 0])))
	print(f'remote public key: {remote_key_pair.public_key}')

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'account_key_link_transaction',
		'linked_public_key': remote_key_pair.public_key,
		'link_action': 'link'
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'account key link transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='account key link transaction')
```
```python
async def create_vrf_key_link(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic VRF account (insecurely deterministically generated for the benefit of related tests)
	# this account will inject randomness into blocks harvested by the (funded) signing account
	vrf_key_pair = facade.KeyPair(PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, 0])))
	print(f'VRF public key: {vrf_key_pair.public_key}')

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'vrf_key_link_transaction',
		'linked_public_key': vrf_key_pair.public_key,
		'link_action': 'link'
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'vrf key link transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='vrf key link transaction')
```

```python
async def create_account_key_unlink(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic remote account (insecurely deterministically generated for the benefit of related tests)
	# this account will sign blocks on behalf of the (funded) signing account
	remote_key_pair = facade.KeyPair(PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, 0])))
	print(f'remote public key: {remote_key_pair.public_key}')

	# when unlinking, linked_public_key must match previous value used in link
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'account_key_link_transaction',
		'linked_public_key': remote_key_pair.public_key,
		'link_action': 'unlink'
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'account key link transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='account key link transaction')
```
```python
async def create_vrf_key_unlink(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic VRF account (insecurely deterministically generated for the benefit of related tests)
	# this account will inject randomness into blocks harvested by the (funded) signing account
	vrf_key_pair = facade.KeyPair(PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, 0])))
	print(f'VRF public key: {vrf_key_pair.public_key}')

	# when unlinking, linked_public_key must match previous value used in link
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'vrf_key_link_transaction',
		'linked_public_key': vrf_key_pair.public_key,
		'link_action': 'unlink'
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'vrf key link transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='vrf key link transaction')
```

#### Voting Links

```python
async def create_voting_key_link(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic root voting public key (insecurely deterministically generated for the benefit of related tests)
	# this account will be participate in voting on behalf of the (funded) signing account
	voting_public_key = PublicKey(signer_key_pair.public_key.bytes[:-4] + bytes([0, 0, 0, 0]))
	print(f'voting public key: {voting_public_key}')

	# notice that voting changes will only take effect after finalization of the block containing the voting key link transaction
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'voting_key_link_transaction',
		'linked_public_key': voting_public_key,
		'start_epoch': 10,
		'end_epoch': 150,
		'link_action': 'link'
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'node key link transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='node key link transaction')
```
```python
async def create_voting_key_unlink(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic root voting public key (insecurely deterministically generated for the benefit of related tests)
	# this account will be participate in voting on behalf of the (funded) signing account
	voting_public_key = PublicKey(signer_key_pair.public_key.bytes[:-4] + bytes([0, 0, 0, 0]))
	print(f'voting public key: {voting_public_key}')

	# notice that voting changes will only take effect after finalization of the block containing the voting key link transaction
	# when unlinking, linked_public_key must match previous value used in link
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'voting_key_link_transaction',
		'linked_public_key': voting_public_key,
		'start_epoch': 10,
		'end_epoch': 150,
		'link_action': 'unlink'
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'node key link transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='node key link transaction')
```

#### Delegated Harvesting Links

```python
async def create_node_key_link(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic node public key (insecurely deterministically generated for the benefit of related tests)
	# this account will be asked to host delegated harvesting of the (funded) signing account
	node_public_key = PublicKey(signer_key_pair.public_key.bytes[:-4] + bytes([0, 0, 0, 0]))
	print(f'node public key: {node_public_key}')

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'node_key_link_transaction',
		'linked_public_key': node_public_key,
		'link_action': 'link'
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'node key link transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='node key link transaction')
```
```python
async def create_node_key_unlink(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic node public key (insecurely deterministically generated for the benefit of related tests)
	# this account will be asked to host delegated harvesting of the (funded) signing account
	node_public_key = PublicKey(signer_key_pair.public_key.bytes[:-4] + bytes([0, 0, 0, 0]))
	print(f'node public key: {node_public_key}')

	# when unlinking, linked_public_key must match previous value used in link
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'node_key_link_transaction',
		'linked_public_key': node_public_key,
		'link_action': 'unlink'
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'node key link transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='node key link transaction')
```

## Blocks
### Harvesting

In Symbol, new blocks are created through a process called *harvesting*.


### Finalization
### Trees
#### Roots & Leaves
#### Merkle Trees
#### Patricia Trees
### Tutorial: Querying Finalization Height

```python
async def get_network_finalized_height():
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint
		async with session.get(f'{SYMBOL_API_ENDPOINT}/chain/info') as response:
			# wait for the (JSON) response
			response_json = await response.json()

			# extract the finalized height from the json
			height = int(response_json['latestFinalizedBlock']['height'])
			print(f'finalized height: {height}')
			return height
```

### Tutorial: Querying Current Height

```python
async def get_network_height():
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint
		async with session.get(f'{SYMBOL_API_ENDPOINT}/chain/info') as response:
			# wait for the (JSON) response
			response_json = await response.json()

			# extract the height from the json
			height = int(response_json['height'])
			print(f'height: {height}')
			return height
```

### Tutorial: Working with Proofs

```python
async def prove_confirmed_transaction(facade, signer_key_pair):
	await _spam_transactions(facade, signer_key_pair, 10)

	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic recipient (it insecurely deterministically generated for the benefit of related tests)
	recipient_address = facade.network.public_key_to_address(PublicKey(signer_key_pair.public_key.bytes[:-4] + bytes([0, 0, 0, 0])))
	print(f'recipient: {recipient_address}')

	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'transfer_transaction',
		'recipient_address': recipient_address,
		'mosaics': [
			{'mosaic_id': generate_mosaic_alias_id('symbol.xym'), 'amount': 1_000000},  # send 1 of XYM to recipient
		],
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'transfer transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='transfer transaction')

	# create a connection to a node
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint to get information about the confirmed transaction
		async with session.get(f'{SYMBOL_API_ENDPOINT}/transactions/confirmed/{transaction_hash}') as response:
			# extract the confirmed block height
			response_json = await response.json()
			confirmed_block_height = int(response_json['meta']['height'])
			print(f'confirmed block height: {confirmed_block_height}')

		# initiate a HTTP GET request to a Symbol REST endpoint to get information about the confirming block
		async with session.get(f'{SYMBOL_API_ENDPOINT}/blocks/{confirmed_block_height}') as response:
			# extract the block transactions hash
			response_json = await response.json()
			block_transactions_hash = Hash256(response_json['block']['transactionsHash'])
			print(f'block transactions hash: {block_transactions_hash}')

		# initiate a HTTP GET request to a Symbol REST endpoint to get a transaction merkle proof
		print(f'{SYMBOL_API_ENDPOINT}/blocks/{confirmed_block_height}/transactions/{transaction_hash}/merkle')
		async with session.get(f'{SYMBOL_API_ENDPOINT}/blocks/{confirmed_block_height}/transactions/{transaction_hash}/merkle') as response:
			# extract the merkle proof path and transform it into format expected by sdk
			response_json = await response.json()
			print(response_json)
			merkle_proof_path = list(map(
				lambda part: MerklePart(Hash256(part['hash']), 'left' == part['position']),
				response_json['merklePath']))
			print(merkle_proof_path)

			# perform the proof
			if prove_merkle(transaction_hash, merkle_proof_path, block_transactions_hash):
				print(f'transaction {transaction_hash} is proven to be in block {confirmed_block_height}')
			else:
				raise RuntimeError(f'transaction {transaction_hash} is NOT proven to be in block {confirmed_block_height}')
```
```python
async def prove_xym_mosaic_state(facade, _):  # pylint: disable=too-many-locals
	# determine the network currency mosaic
	network_currency_id = await get_network_currency()
	network_currency_id_formatted = f'{network_currency_id:08X}'

	# look up the properties of the network currency mosaic
	mosaic_properties_json = (await get_mosaic_properties(network_currency_id_formatted))['mosaic']
	print(mosaic_properties_json)

	# serialize and hash the mosaic properties
	writer = BufferWriter()
	writer.write_int(int(mosaic_properties_json['version']), 2)
	writer.write_int(int(mosaic_properties_json['id'], 16), 8)
	writer.write_int(int(mosaic_properties_json['supply']), 8)
	writer.write_int(int(mosaic_properties_json['startHeight']), 8)
	writer.write_bytes(facade.Address(unhexlify(mosaic_properties_json['ownerAddress'])).bytes)
	writer.write_int(int(mosaic_properties_json['revision']), 4)
	writer.write_int(int(mosaic_properties_json['flags']), 1)
	writer.write_int(int(mosaic_properties_json['divisibility']), 1)
	writer.write_int(int(mosaic_properties_json['duration']), 8)
	mosaic_hashed_value = Hash256(sha3.sha3_256(writer.buffer).digest())
	print(f'mosaic hashed value: {mosaic_hashed_value}')

	# hash the mosaic id to get the key
	writer = BufferWriter()
	writer.write_int(int(mosaic_properties_json['id'], 16), 8)
	mosaic_encoded_key = Hash256(sha3.sha3_256(writer.buffer).digest())
	print(f'mosaic encoded key: {mosaic_encoded_key}')

	# get the current network height
	network_height = await get_network_height()

	# create a connection to a node
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint to get information about the last block
		async with session.get(f'{SYMBOL_API_ENDPOINT}/blocks/{network_height}') as response:
			# extract the sub cache merkle roots and the stateHash
			response_json = await response.json()
			state_hash = Hash256(response_json['block']['stateHash'])
			subcache_merkle_roots = [Hash256(root) for root in response_json['meta']['stateHashSubCacheMerkleRoots']]
			print(f'state hash: {state_hash}')
			print(f'subcache merkle roots: {subcache_merkle_roots}')

		# initiate a HTTP GET request to a Symbol REST endpoint to get a state hash merkle proof
		async with session.get(f'{SYMBOL_API_ENDPOINT}/mosaics/{network_currency_id_formatted}/merkle') as response:
			# extract the merkle proof and transform it into format expected by sdk
			response_json = await response.json()
			merkle_proof_path = deserialize_patricia_tree_nodes(unhexlify(response_json['raw']))

			# perform the proof
			proof_result = prove_patricia_merkle(
				mosaic_encoded_key,
				mosaic_hashed_value,
				merkle_proof_path,
				state_hash,
				subcache_merkle_roots)
			print(f'mosaic {network_currency_id_formatted} proof concluded with {proof_result}')
```

## XYM

XYM is the base-layer currency of Symbol. Any activity on the blockchain - from recording data to sending assets or messages - requires a small fee of XYM, which rewards validators who create and finalize blocks.

### Tutorial: Querying XYM Supply
Maximum supply is the maximum number of XYM that can ever be minted.
```sh
curl https://${SYMBOL_API_NODE}:3001/network/currency/supply/max
```
```python
async def get_maximum_supply():
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint
		async with session.get(f'{SYMBOL_API_ENDPOINT}/network/currency/supply/max') as response:
			# wait for the (text) response and interpret it as a floating point value
			maximum_supply = float(await response.text())
			print(f'maximum supply: {maximum_supply:.6f} XYM')
			return maximum_supply
```

Total supply is the number of XYM minted to date.
```sh
curl https://${SYMBOL_API_NODE}:3001/network/currency/supply/total
```
```python
async def get_total_supply():
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint
		async with session.get(f'{SYMBOL_API_ENDPOINT}/network/currency/supply/total') as response:
			# wait for the (text) response and interpret it as a floating point value
			total_supply = float(await response.text())
			print(f'total supply: {total_supply:.6f} XYM')
			return total_supply
```

Circulating supply is the number of XYM minted to date, excluding the balances of the two fee sinks:
1. VORTEX4: Mosaic and namespace rental fee sink
2. VORTEX3: Network fee sink

```sh
curl https://${SYMBOL_API_NODE}:3001/network/currency/supply/circulating
```
```python
async def get_circulating_supply():
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint
		async with session.get(f'{SYMBOL_API_ENDPOINT}/network/currency/supply/circulating') as response:
			# wait for the (text) response and interpret it as a floating point value
			circulating_supply = float(await response.text())
			print(f'circulating supply: {circulating_supply:.6f} XYM')
			return circulating_supply
```

### Tutorial: Querying Current Block Rewards

:warning: this is not currently supported by REST; best we can do is read reward from CURRENT block

### Tutorial: Querying Historical Block Rewards

:warning: we can get this from block height, is that desire?

## Advanced topics

### Cross-chain swaps

Symbol's _secret lock_ and _secret proof_ can be compared to hashed timelock contract in other blockchains. Thanks to them cross-chain swaps can be made.

#### Example: ETH⟷XYM cross-chain swap

To make cross chain swap with Ethereum there's a contract needed. Exemplary contract is available in https://github.com/gimre-xymcity/hashed-timelock-contract-ethereum.

:::warning
This contract is not production ready.
Few things that should be taken care of:
 * timing lock/contract expiry is crucial for security of both parties (see explanation after _Guts_)
 * due to gas involved, ETH contract should use sha3 instead of double sha256,
 * unfortunatelly, sha3 opcode in EVM is actually keccak256,
 * although Symbol uses sha3 it does not older variant usually referred to as keccak, `Op_Sha3_256` lock type is actual sha3 not keccak; adding keccak would require fork
:::

Contract has been deployed on Sepolia testnet under address [0xd58e030bd21c7788897aE5Ea845DaBA936e91D2B](https://sepolia.etherscan.io/address/0xd58e030bd21c7788897ae5ea845daba936e91d2b).

The contract contains following methods, that will be used:
 1. `newContract(address receiver, bytes32 lock, uint timelock)` - creates new lock where destination account is `receiver`, `lock` is a secret lock, and `timelock` is unix epoch-based timestap limit, until which withdrawal is possible - this is almost 1-to-1 Symbol's `secret lock transaction`
 2. `withdraw(bytes32 contractId, bytes preimage)` - allows to withdraw funds given valid `preimage`; note, that this method must be called by whoever is `receiver` in 1. - this is similar to Symbol's `secret proof transaction`, although Symbol allows sending it from unrelated account, but it's still `receiver` that will get the funds
 3. `refund(bytes32 contractId)` - available after lock expires, only owner of a lock can call it to refund what's inside the lock - in Symbol refund happens automatically after lock expires and generates `BalanceChangeReceipt` within a block with type `LockSecret_Expired`.

There are multiple scenarios, how cross-chain swap scenario can look like, the following is just an example. We have two parties Alice and Bob, with following addresses:

| | ETH | Symbol |
|--- |--- |--- |
| Alice | **0xa11ce**c3497B522a25c08Dd45Cc07663311E04f10 | **TALICE**CI35BNIJQA5CNUKI2DY3SXNEHPZJSOVAA |
| Bob | **0xb0bb1**2D1befe54Dc773CefE7BB3687c72a33d335 | **TBOBBY**KOYQBWK3HSX7NQVJ5JFPE22352AVDXXAA |

**Scenario:**
 1. in Ethereum: **0xa11ce** creates a new lock via `newContract` call with `receiver` = **0xb0bb1**, only Alice knows the preimage (proof) to a used `lock`
 2. in Symbol: **TBOBBY** creates _secret lock_ with recipient set to **TALICE**, Bob is using SAME lock value as Alice;<br> note, this lock should have duration that will expire a bit before lock created by Alice in Ethereum
 3. in Symbol: Alice withdrawals funds by issuing _secret proof_ with a proper preimage
 4. in Ethereum: Bob learned preimage from proof published in Symbol network so he can now call `withdraw` in ethereum

**Guts:**

Finally some hands-on. For ethereum, example below will use tools from [foundry](https://github.com/foundry-rs/foundry) toolkit.

Alice will swap 0.2 ETH with Bob for 7887 XYM.

1. Alice creates a lock:<br>`cast send --value 0.2ether 0xd58e030bd21c7788897aE5Ea845DaBA936e91D2B 'newContract(address,bytes32,uint)' 0xb0bb12D1befe54Dc773CefE7BB3687c72a33d335 b867db875479bcc0287352cdaa4a1755689b8338777d0915e9acd9f6edbc96cb 1663568700`
	1. [0xd58e030bd21c7788897aE5Ea845DaBA936e91D2B](https://sepolia.etherscan.io/address/0xd58e030bd21c7788897ae5ea845daba936e91d2b) is the contract address mentioned earlier
	2. **0xb0bb1**2D1befe54Dc773CefE7BB3687c72a33d335 - is bob eth destination,
	3. b867…96cb - is the lock value,
	4. finally 1663568700 is a timestamp corresponding to _Monday, 19 September 2022 06:25:00_ GMT,
	5. corresponding tx on Sepolia testnet: [0x23265e1a9aaaa70d582369fd3edbbe20b2f44a3a18a0a96205fb8beac8689964](https://sepolia.etherscan.io/tx/0x23265e1a9aaaa70d582369fd3edbbe20b2f44a3a18a0a96205fb8beac8689964),
	6. as can be seen in transaction logs, `contractId` (read: identifier of created lock) is `0x81b0f164348bb17de94cca31b8d41ce435321aa2bb5721eb5c90cadd886e4c3f`
2. Bob creates lock inside Symbol:
	1. First bob needs to turn Alice's timelock into secret lock duration.
	2. Current testnet block at the time of writing is **697357**, with network timestamp: **25407256928**.
	3. Alice's unix epoch-based timestamp needs to be converted to Symbol network timestamp, additionally, we want to lower it by two hours, so that lock in Symbol expires prior to corresponding lock in eth (see explanation below)
	   ```python
	   timelock_datetime = datetime.fromtimestamp(unix_epoch_timelock, tz=timezone.utc)
	   symbol_timestamp = facade.network.from_datetime(timelock_datetime)
	   symbol_timestamp.add_hours(-2)
	   ```
	4. Produced network timestamp needs to be turned into block-based duration. Network timestamps are in milliseconds, so difference needs to be divided by 1000. Symbol testnet network block generation target time is 30s, so to obtain number of blocks:
	   ```python
	   duration = int((symbol_timestamp.timestamp - 25407256928) / 1000 / 30)
	   # (25719853000 - 25407256928) / 1000 / 30 = 10419
	   ```
	5. Finally Bob can create secret lock
		```python
		async def create_secret_lock(facade, signer_key_pair):
			# derive the signer's address
			signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
			print(f'creating transaction with signer {signer_address}')
		
			# get the current network time from the network, and set the transaction deadline two hours in the future
			network_time = await get_network_time()
			network_time = network_time.add_hours(2)
		
			# create a deterministic recipient (it insecurely deterministically generated for the benefit of related tests)
			recipient_address = facade.network.public_key_to_address(PublicKey(signer_key_pair.public_key.bytes[:-4] + bytes([0, 0, 0, 0])))
			print(f'recipient: {recipient_address}')
		
			# double sha256 hash the proof value
			secret_hash = Hash256(hashlib.sha256(hashlib.sha256('correct horse battery staple'.encode('utf8')).digest()).digest())
		
			transaction = facade.transaction_factory.create({
				'signer_public_key': signer_key_pair.public_key,
				'deadline': network_time.timestamp,
		
				'type': 'secret_lock_transaction',
				'mosaic': {'mosaic_id': generate_mosaic_alias_id('symbol.xym'), 'amount': 7_000000},  # mosaic to transfer upon proof
		
				'duration': 111,  # number of blocks
				'recipient_address': recipient_address,
				'secret': secret_hash,
				'hash_algorithm': 'hash_256'  # double Hash256
			})
		
			# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
			transaction.fee = Amount(100 * transaction.size)
		
			# sign the transaction and attach its signature
			signature = facade.sign_transaction(signer_key_pair, transaction)
			facade.transaction_factory.attach_signature(transaction, signature)
		
			# hash the transaction (this is dependent on the signature)
			transaction_hash = facade.hash_transaction(transaction)
			print(f'secret lock transaction hash {transaction_hash}')
		
			# finally, construct the over wire payload
			json_payload = facade.transaction_factory.attach_signature(transaction, signature)
		
			# print the signed transaction, including its signature
			print(transaction)
		
			# submit the transaction to the network
			async with ClientSession(raise_for_status=True) as session:
				# initiate a HTTP PUT request to a Symbol REST endpoint
				async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
					response_json = await response.json()
					print(f'/transactions: {response_json}')
		
			# wait for the transaction to be confirmed
			await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='secret lock transaction')
		```
3. Now Alice can claim the lock, that part is substantially easier:
	1. create secret proof (withdraw)
		```python
		async def create_secret_proof(facade, signer_key_pair):
			# derive the signer's address
			signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
			print(f'creating transaction with signer {signer_address}')
		
			# get the current network time from the network, and set the transaction deadline two hours in the future
			network_time = await get_network_time()
			network_time = network_time.add_hours(2)
		
			# create a deterministic recipient (it insecurely deterministically generated for the benefit of related tests)
			recipient_address = facade.network.public_key_to_address(PublicKey(signer_key_pair.public_key.bytes[:-4] + bytes([0, 0, 0, 0])))
			print(f'recipient: {recipient_address}')
		
			# double sha256 hash the proof value
			secret_hash = Hash256(hashlib.sha256(hashlib.sha256('correct horse battery staple'.encode('utf8')).digest()).digest())
		
			transaction = facade.transaction_factory.create({
				'signer_public_key': signer_key_pair.public_key,
				'deadline': network_time.timestamp,
		
				'type': 'secret_proof_transaction',
		
				'recipient_address': recipient_address,
				'secret': secret_hash,
				'hash_algorithm': 'hash_256',  # double Hash256
				'proof': 'correct horse battery staple'
			})
		
			# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
			transaction.fee = Amount(100 * transaction.size)
		
			# sign the transaction and attach its signature
			signature = facade.sign_transaction(signer_key_pair, transaction)
			facade.transaction_factory.attach_signature(transaction, signature)
		
			# hash the transaction (this is dependent on the signature)
			transaction_hash = facade.hash_transaction(transaction)
			print(f'secret proof transaction hash {transaction_hash}')
		
			# finally, construct the over wire payload
			json_payload = facade.transaction_factory.attach_signature(transaction, signature)
		
			# print the signed transaction, including its signature
			print(transaction)
		
			# submit the transaction to the network
			async with ClientSession(raise_for_status=True) as session:
				# initiate a HTTP PUT request to a Symbol REST endpoint
				async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
					response_json = await response.json()
					print(f'/transactions: {response_json}')
		
			# wait for the transaction to be confirmed
			await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='secret proof transaction')
		```
4. Now that Bob has learned super complicated proof he can use contract's `withdraw` method:<br>`cast send 0xd58e030bd21c7788897aE5Ea845DaBA936e91D2B 'withdraw(bytes32, bytes)' 0x81b0f164348bb17de94cca31b8d41ce435321aa2bb5721eb5c90cadd886e4c3f 636F727265637420686F727365206261747465727920737461706C65`
	1. `0xd58e030bd21c7788897aE5Ea845DaBA936e91D2B` is a contract address, the same one as used by Alice,
	2. `0x81b0f164348bb17de94cca31b8d41ce435321aa2bb5721eb5c90cadd886e4c3f` is a lock contract id (`contractId`),
	3. `636F727265637420686F727365206261747465727920737461706C65` - i.e. found in Symbol's testnet explorer (from earlier),
	4. corresponding transaction on Sepolia testnet: `0x14eef724a76ae2aa29b0c405cbc0da2af3c7827e198bfdbbdadbb27eb67a2c05`
5. And they lived happily ever after.

As mentioned in 2.3 lock created in symbol should be _slightly_ shorter. If it would be longer, or in general if ETH timelock will expire before Symbol's lock, Alice could cheat Bob, simply by waiting until eth timelock expires and then publishing both:
 * withdraw (secret proof transaction) inside Symbol network
 * calling `refund(...)` method indside Ethereum network.

TODO: hands-on example with expired locks

## Catapult

Catapult is the reference client for Symbol. Written in C++, Catapult's key innovation is composibility.

### Architecture

### Plugins

Each feature that makes up Symbol is defined as a *plugin*. All nodes within same network need to share same set of plugins.

:warning: what should go here?

### Transaction Plugins

#### Account_Link

#### Aggregate

#### Lock_Hash

#### Lock_Secret

#### Lock_Shared

#### Metadata

#### Mosaic

#### Multisig

#### Namespace

#### Restriction_Account

#### Restriction_Mosaic

#### Transfer

### Core Plugins

#### Importance
#### Observers
#### Voting
#### Validators

### Services

#### HashCache
#### Signature

### Configuration
### Tutorial: Working with MongoDB
### Tutorial: Working with REST

:warning: are existing REST docs good enough?

### Tutorial: Working with Websockets

```python
async def read_websocket_block(_, _1):
	# connect to websocket endpoint
	async with connect(SYMBOL_WEBSOCKET_ENDPOINT) as websocket:
		# extract user id from connect response
		response_json = json.loads(await websocket.recv())
		user_id = response_json['uid']
		print(f'established websocket connection with user id {user_id}')

		# subscribe to block messages
		subscribe_message = {'uid': user_id, 'subscribe': 'block'}
		await websocket.send(json.dumps(subscribe_message))
		print('subscribed to block messages')

		# wait for the next block message
		response_json = json.loads(await websocket.recv())
		print(f'received message with topic: {response_json["topic"]}')
		print(f'received block at height {response_json["data"]["block"]["height"]} with hash {response_json["data"]["meta"]["hash"]}')
		print(response_json['data']['block'])

		# unsubscribe from block messages
		unsubscribe_message = {'uid': user_id, 'unsubscribe': 'block'}
		await websocket.send(json.dumps(unsubscribe_message))
		print('unsubscribed from block messages')
```
```python
async def read_websocket_transaction_flow(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# connect to websocket endpoint
	async with connect(SYMBOL_WEBSOCKET_ENDPOINT) as websocket:
		# extract user id from connect response
		response_json = json.loads(await websocket.recv())
		user_id = response_json['uid']
		print(f'established websocket connection with user id {user_id}')

		# subscribe to transaction messages associated with the signer
		# * confirmedAdded - transaction was confirmed
		# * unconfirmedAdded - transaction was added to unconfirmed cache
		# * unconfirmedRemoved - transaction was removed from unconfirmed cache
		# notice that all of these are scoped to a single address
		channel_names = ('confirmedAdded', 'unconfirmedAdded', 'unconfirmedRemoved')
		for channel_name in channel_names:
			subscribe_message = {'uid': user_id, 'subscribe': f'{channel_name}/{signer_address}'}
			await websocket.send(json.dumps(subscribe_message))
			print(f'subscribed to {channel_name} messages')

		# send two transactions
		unconfirmed_transactions_count = 2
		await _spam_transactions(facade, signer_key_pair, unconfirmed_transactions_count)

		# read messages from the websocket as the transactions move from unconfirmed to confirmed
		# notice that "added" messages contain the full transaction payload whereas "removed" messages only contain the hash
		# expected progression is unconfirmedAdded, unconfirmedRemoved, confirmedAdded
		while True:
			response_json = json.loads(await websocket.recv())
			topic = response_json['topic']
			print(f'received message with topic {topic} for transaction {response_json["data"]["meta"]["hash"]}')
			if topic.startswith('confirmedAdded'):
				unconfirmed_transactions_count -= 1
				if 0 == unconfirmed_transactions_count:
					print('all transactions confirmed')
					break

		# unsubscribe from transaction messages
		for channel_name in channel_names:
			unsubscribe_message = {'uid': user_id, 'unsubscribe': f'{channel_name}/{signer_address}'}
			await websocket.send(json.dumps(unsubscribe_message))
			print(f'unsubscribed from {channel_name} messages')
```
```python
async def read_websocket_transaction_bonded_flow(facade, signer_key_pair):
	# pylint: disable=too-many-locals
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create cosignatory key pairs, where each cosignatory will be required to cosign initial modification
	# (they are insecurely deterministically generated for the benefit of related tests)
	cosignatory_key_pairs = [facade.KeyPair(PrivateKey(signer_key_pair.private_key.bytes[:-4] + bytes([0, 0, 0, i]))) for i in range(3)]
	cosignatory_addresses = [facade.network.public_key_to_address(key_pair.public_key) for key_pair in cosignatory_key_pairs]

	embedded_transactions = [
		facade.transaction_factory.create_embedded({
			'type': 'multisig_account_modification_transaction',
			'signer_public_key': signer_key_pair.public_key,

			'min_approval_delta': 2,  # number of signatures required to make any transaction
			'min_removal_delta': 2,  # number of signatures needed to remove a cosignatory from multisig
			'address_additions': cosignatory_addresses
		})
	]

	# create the transaction, notice that signer account that will be turned into multisig is a signer of transaction
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'aggregate_bonded_transaction',
		'transactions_hash': facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'multisig account modification bonded (new) {transaction_hash}')

	# print the signed transaction, including its signature
	print(transaction)

	# create a hash lock transaction to allow the network to collect cosignaatures for the aggregate
	await create_hash_lock(facade, signer_key_pair, transaction_hash)

	# connect to websocket endpoint
	async with connect(SYMBOL_WEBSOCKET_ENDPOINT) as websocket:
		# extract user id from connect response
		response_json = json.loads(await websocket.recv())
		user_id = response_json['uid']
		print(f'established websocket connection with user id {user_id}')

		# subscribe to transaction messages associated with the signer
		# * confirmedAdded - transaction was confirmed
		# * unconfirmedAdded - transaction was added to unconfirmed cache
		# * unconfirmedRemoved - transaction was removed from unconfirmed cache
		# * partialAdded - transaction was added to partial cache
		# * partialRemoved - transaction was removed from partial cache
		# * cosignature - cosignature was added
		# notice that all of these are scoped to a single address
		channel_names = ('confirmedAdded', 'unconfirmedAdded', 'unconfirmedRemoved', 'partialAdded', 'partialRemoved', 'cosignature')
		for channel_name in channel_names:
			subscribe_message = {'uid': user_id, 'subscribe': f'{channel_name}/{signer_address}'}
			await websocket.send(json.dumps(subscribe_message))
			print(f'subscribed to {channel_name} messages')

		# submit the partial (bonded) transaction to the network
		async with ClientSession(raise_for_status=True) as session:
			# initiate a HTTP PUT request to a Symbol REST endpoint
			async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions/partial', json=json.loads(json_payload)) as response:
				response_json = await response.json()
				print(f'/transactions/partial: {response_json}')

			# wait for the partial transaction to be cached by the network (this should be partialAdded)
			response_json = json.loads(await websocket.recv())
			print(f'received message with topic {response_json["topic"]} for transaction {response_json["data"]["meta"]["hash"]}')

			# submit the (detached) cosignatures to the network
			for cosignatory_key_pair in cosignatory_key_pairs:
				cosignature = facade.cosign_transaction(cosignatory_key_pair, transaction, True)
				cosignature_json_payload = json.dumps({
					'version': str(cosignature.version),
					'signerPublicKey': str(cosignature.signer_public_key),
					'signature': str(cosignature.signature),
					'parentHash': str(cosignature.parent_hash)
				})
				print(cosignature_json_payload)

				# initiate a HTTP PUT request to a Symbol REST endpoint
				async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions/cosignature', json=json.loads(cosignature_json_payload)) as response:
					response_json = await response.json()
					print(f'/transactions/cosignature: {response_json}')

		# read messages from the websocket as the transaction moves from partial to unconfirmed to confirmed
		# notice that "added" messages contain the full transaction payload whereas "removed" messages only contain the hash
		# expected progression is cosignature, cosignature, partialRemoved, unconfirmedAdded, unconfirmedRemoved, confirmedAdded
		while True:
			response_json = json.loads(await websocket.recv())
			topic = response_json['topic']
			if topic.startswith('cosignature'):
				cosignature = response_json['data']
				print(f'received cosignature for transaction {cosignature["parentHash"]} from {cosignature["signerPublicKey"]}')
			else:
				print(f'received message with topic {topic} for transaction {response_json["data"]["meta"]["hash"]}')

			if topic.startswith('confirmedAdded'):
				print('transaction confirmed')
				break

		# unsubscribe from transaction messages
		for channel_name in channel_names:
			unsubscribe_message = {'uid': user_id, 'unsubscribe': f'{channel_name}/{signer_address}'}
			await websocket.send(json.dumps(unsubscribe_message))
			print(f'unsubscribed from {channel_name} messages')
```
```python
async def read_websocket_transaction_error(facade, signer_key_pair):
	# pylint: disable=too-many-locals
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# connect to websocket endpoint
	async with connect(SYMBOL_WEBSOCKET_ENDPOINT) as websocket:
		# extract user id from connect response
		response_json = json.loads(await websocket.recv())
		user_id = response_json['uid']
		print(f'established websocket connection with user id {user_id}')

		# subscribe to transaction messages associated with the signer
		# * status - transaction was rejected
		# notice that all of these are scoped to a single address
		subscribe_message = {'uid': user_id, 'subscribe': f'status/{signer_address}'}
		await websocket.send(json.dumps(subscribe_message))
		print('subscribed to status messages')

		# create a deterministic recipient (it insecurely deterministically generated for the benefit of related tests)
		recipient_address = facade.network.public_key_to_address(PublicKey(signer_key_pair.public_key.bytes[:-4] + bytes([0, 0, 0, 0])))
		print(f'recipient: {recipient_address}')

		# derive the signer's address
		signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
		print(f'creating transaction with signer {signer_address}')

		# get the current network time from the network, and set the transaction deadline two hours in the future
		network_time = await get_network_time()
		network_time = network_time.add_hours(2)

		# prepare transaction that will be rejected (insufficient balance)
		transaction = facade.transaction_factory.create({
			'signer_public_key': signer_key_pair.public_key,
			'deadline': network_time.timestamp,

			'type': 'transfer_transaction',
			'recipient_address': recipient_address,
			'mosaics': [
				{'mosaic_id': generate_mosaic_alias_id('symbol.xym'), 'amount': 1000_000000}
			],
		})

		# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
		transaction.fee = Amount(100 * transaction.size)

		# sign the transaction and attach its signature
		signature = facade.sign_transaction(signer_key_pair, transaction)
		facade.transaction_factory.attach_signature(transaction, signature)

		# hash the transaction (this is dependent on the signature)
		transaction_hash = facade.hash_transaction(transaction)
		print(f'transfer transaction hash {transaction_hash}')

		# finally, construct the over wire payload
		json_payload = facade.transaction_factory.attach_signature(transaction, signature)

		# print the signed transaction, including its signature
		print(transaction)

		# submit the transaction to the network
		async with ClientSession(raise_for_status=True) as session:
			# initiate a HTTP PUT request to a Symbol REST endpoint
			async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
				response_json = await response.json()
				print(f'/transactions: {response_json}')

		# read messages from the websocket as the transactions move from unconfirmed to confirmed
		# notice that "added" messages contain the full transaction payload whereas "removed" messages only contain the hash
		# expected progression is unconfirmedAdded, unconfirmedRemoved, confirmedAdded
		response_json = json.loads(await websocket.recv())
		print(f'received message with topic: {response_json["topic"]}')
		print(f'transaction {response_json["data"]["hash"]} was rejected with {response_json["data"]["code"]}')

		# unsubscribe from status messages
		unsubscribe_message = {'uid': user_id, 'unsubscribe': f'status/{signer_address}'}
		await websocket.send(json.dumps(unsubscribe_message))
		print('unsubscribed from status messages')
```

TODO - add finalizedBlock example when network is restarted

## Catbuffer

### Introduction to DSLs

# CATS DSL

CATS are composed of three primary declarations: `using`, `enum`, `struct`.
Schemas are whitespace significant.

## import

CATS files can include other files by using the `import` statement.
For example, to import a file called "other.cats":
```cpp
import "other.cats"
```

All imported filenames are relative to the include path passed to the parser.
Using the CLI tool, this can be specified by the `--include` command line argument.

## using

Alias statements can be used to name a unique POD type.
CATS supports two types of built-ins:

- Integer types: unsigned ("uint") and signed ("int") for sizes { 1, 2, 4, 8 }.
For example, to define a `Height` type that is represented by a 8 byte unsigned integer:
```cpp
using Height = uint64
```

- Fixed buffer types: unsigned byte buffer of any length.
For example, to define a `PublicKey` type that is represented by a 32 byte unsigned buffer:
```cpp
using PublicKey = binary_fixed(32)
```

Importantly, each alias will be treated as a unique type and cannot be used interchangeably.
For example, `Weight` cannot be used where `Height` is expected and vice versa.
```cpp
using Height = uint64
using Weight = uint64
```

## enum

Enumeration statements can be used to define a set of possible values.
Each enumeration specifies an integer backing type.

For example, to define a `TransportMode` enumeration backed by a 2 byte unsigned integer:
```cpp
enum TransportMode : uint16
```

Values making up an enumeration follow the enumeration declaration on indented lines.
For example, to add three values to the `TransportMode` enumeration named `ROAD` (value 0x0001) and `SEA` (0x0002) and `SKY` (0x0004):
```cpp
enum TransportMode : uint32
	ROAD = 0x0001
	SEA = 0x0002
	SKY = 0x0004
```

### attributes

Hints can be attached to enumerations using attributes.

Enumerations support the following attributes:
1. `is_bitwise`: indicates that the enumeration represents flags and should support bitwise operations.

For example, to set the `is_bitwise` attribute on the `TransportMode` enumeration:
```cpp
@is_bitwise
enum TransportMode : uint32
	ROAD = 0x0001
	SEA = 0x0002
	SKY = 0x0004
```

## struct

Structure statements are used to define structured binary payloads.
Structure definition are comprehensive.
Unlike other formats, the CATS parser will never add extraneous data or padding.

Structures can have any of the following modifiers:
1. None: Generators are recommended to include the type in final output.
2. abstract: Generators are recommended to include the type in final output and produce corresponding factory.
3. inline: Generators are recommended to discard the structure from final output.

For example, to define a `Vehicle` struct with the `abstract` modifier:
```cpp
abstract struct Vehicle
```

Fields making up a structure follow the structure declaration on indented lines.
For example, to add an 8 byte unsigned  `weight` field of type to the `Vehicle` structure:
```cpp
abstract struct Vehicle
	weight = uint32
```

`make_const` can be used to define a const field, which does not appear in the struct layout.
For example, to define a 2 byte unsigned constant `TRANSPORT_MODE` with value `ROAD`:
```cpp
struct Car
	TRANSPORT_MODE = make_const(TransportMode, ROAD)
```

`make_reserved` can be used to define a reserved field, which does appear in the layout and specifies a default value.
For example, to define a 1 byte unsigned constant `wheel_count` with value 4:
```cpp
inline struct Car
	wheel_count = make_reserved(uint8, 4)
```

`sizeof` can be used to define a field that is filled with the size of another field.
For example, to define a 2 byte unsigned `car_size` that is filled with the size of the `car` field:
```cpp
inline struct SingleCarGarage
	car_size = sizeof(uint16, car)

	car = Car
```

### conditionals

Fields can be made conditional on the values of other fields.
The approximates the union concept present in some languages.
CATS supports the following operators:
1. `equals`: conditional field is included if reference field value matches condition value exactly
1. `not equals`: conditional field is included if reference field value does NOT match condition value
1. `has`: conditional field is included if reference field value has all of the condition flags set
1. `not has`: conditional field is included if reference field value does NOT have all of the condition flags set

For example, to indicate `buoyancy` is only present when `transport_mode` is equal to `SEA`:
```cpp
abstract struct Vehicle
	transport_mode = TransportMode

	buoyancy = uint32 if SEA equals transport_mode
```

### arrays

Dynamically sized arrays are supported.
Each array has an associated size that can be a constant, a property reference or a special `__FILL__` keyword.

For example, to define a `Garage` with a `vehicles` field that is composed of `vehicles_count` `Vehicle` structures:
```cpp
struct Garage
	vehicles_count = uint32

	vehicles = array(Vehicle, vehicles_count)
```

The special `__FILL__` keyword indicates that the array extends until the end of the structure.
In order for `__FILL__` to be used, the containing structure must contain a field containing its size in bytes, specified via the `@size` attribute.
For example, to indicate the `vehicles` array composed of `Vehicle` structures extends to the end of the `Garage` structure with byte size `garage_byte_size`:
```cpp
@size(garage_byte_size)
struct Garage
	garage_byte_size = uint32

	vehicles = array(Vehicle, __FILL__)
```

:warning: Array element types (`Vehicle` used in the examples) must either be fixed sized structures or variable sized structures with a `@size` attribute attached.

### inlines
A structure can be inlined within another using the `inline` keyword.
For example, to inline `Vehicle` at the start of a `Car` structure with two fields:
```cpp
struct Car
	inline Vehicle

	max_clearance = Height
	has_left_steering_wheel = uint8
```

Inlines are expanded where they appear, so the order of `Car` fields will be: {weight, max_clearance, has_left_steering_wheel}.
The expansion will be equivalent to:
```cpp
struct Car
	weight = uint32

	max_clearance = Height
	has_left_steering_wheel = uint8
```

In addition, a named inline will inline a referenced structure's fields with a prefix.
For example, in the following `SizePrefixedString` is inlined in `Vehicle` as `friendly_name`:
```cpp
inline struct SizePrefixedString
	size = uint32
	__value__ = array(int8, size)

abstract struct Vehicle
	weight = uint32

	friendly_name = inline SizePrefixedString

	year = uint16
```

The expansion will be equivalent to:
```cpp
abstract struct Vehicle
	weight = uint32

	friendly_name_size = uint32
	friendly_name = array(int8, friendly_name_size)

	year = uint16
```

Within the inlined structure, `__value__` is a special field name that will be replaced with the name (`friendly_name`) used in the containing structure (`Vehicle`).
All other fields in the inlined structure will have names prepended with the name (`friendly_name`) used in the containing structure (`Vehicle`) and an underscore.
So, `__value__` becomes `friendly_name` and `size` becomes  `friendly_name ` + `_` + `size` or `friendly_name_size`.

### attributes

Hints can be attached to structures using attributes.

Structures support the following attributes:
1. `is_aligned`: indicates that all structure fields are positioned on aligned boundaries.
1. `is_size_implicit`: indicates that the structure could be referenced in a `sizeof(x)` statement and must support a size calculation.
1. `size(x)`: indicates that the `x` field contains the full size of the (variable sized) structure.
1. `initializes(x, Y)`: indicates that the `x` field should be initialized with the `Y` constant.
1. `discriminator(x [, y]+)`: indicates that the (`x`, ...`y`) properties should be used as the discriminator when generating a factory (only has meaning for abstract structures).
1. `comparer(x [!a] [, y [!b]])`: indicates that the (`x`, ...`y`) properties should be used for custom sorting. optional (`a`, ...` b`) transforms can be specified and applied prior to property comparison. currently, the only transform supported is `ripemd_keccak_256` for backwards compatibility with NEM.

For example, to link the `transport_mode` field with the `TRANSPORT_MODE` constant:
```cpp
@initializes(transport_mode, TRANSPORT_MODE)
abstract struct Vehicle
	transport_mode = TransportMode

struct Car
	TRANSPORT_MODE = make_const(TransportMode, ROAD)

	inline Vehicle
```

Notice that `TRANSPORT_MODE` can be defined in any derived structure.

Array fields support the following attributes:
1. `is_byte_constrained`: indicates the size value should be interpreted as a byte value instead of an element count.
1. `alignment(x [, [not] pad_last])`: indicates that elements should be padded so that they start on `x`-aligned boundaries.
1. `sort_key(x)`: indicates that elements within the array should be sorted by the `x` property.

When alignment is specified, by default, the final element is padded to end on an `x`-aligned boundary.
This can be made explicit by including the `pad_last` qualifier.
This can be disabled by including the `not pad_last` qualifier, which will not pad the last element to an `x`-aligned boundary.

For example, to sort vehicles by `weight`:
```cpp
struct Garage
	@sort_key(weight)
	vehicles = array(Vehicle, __FILL__)
```

Integer fields support the following attribute:
1. `sizeref(x [, y])`: indicates the field should be initialized with the size of the `x` property adjusted by `y`.

For example, to autopopulate `vehicle_size` with the size of itself and the vehicle field:
```cpp
struct Garage
	@sizeref(vehicle, 2)
	vehicle_size = uint16

	vehicle = Vehicle
```

## comments

Any line starting with a `#` is treated as a comment.
If a comment line is directly above a declaration or sub-declaration it is treated as documentation and preserved by the parser.
Otherwise, it is discarded.

For example, in the following "comment 1" is discarded while "comment 2 comment 3" is extracted as the documentation for Height.
```python
# comment 1

# comment 2
# comment 3
using Height = uint64
```

### Schemas

## Analytics

### Querying Network Mosaics

:warning: what examples are desired here?

### Tutorial: Querying Total Mosaics Issued on Symbol

:warning: i don't think we can do this via REST but can via mongo

### Tutorial: Querying Total Nodes Active

:warning: a node can only return nodes it knows about from its view. is that intent or should example crawl.

### Tutorial: Working with Block (.blk) Data

Catapult is storing blocks within data directory. There are two optimizations:
 * catapult stores multiple blocks in a single file, this is dependent on `fileDatabaseBatchSize` setting within config-node.configuration, by default it is set to 100
 * catapult stores block files (and statements) within subdirectories, there are at most 10_000 _objects_ per directory, that means that with setting above, there will be 100 block files within single subirectory.

Every block file starts with a small header, that contains absolute ofsset within a file to start of a block.

Following examples, will show how to find files and start offsets of few blocks, given settings above.

Example 1. Nemesis block

 1. Nemesis block has height = 1, first height needs to be rounded to a batch file id using `fileDatabaseBatchSize`, `(1 / fileDatabaseBatchSize) * fileDatabaseBatchSize` = 0
 2. next the path to actual batch file is `<directory>/<filename>.dat`, where `directory` is `id / 10_000` and `filename` is `id % 10_000`, both are padded with additional zeroes;
In this trivial case, this results in path `00000/00000.dat`
 3. The header contains 100 8-byte words, with offsets for blocks 0-99, there is no block with height 0, so the entry contains 0, offset to nemesis block will be second word
 (and with default settings mentioned above = 0x320)

Example 2. Block 1690553 (1.0.3.4 protocol fork height)
 1. round to batch file id: `(1690553 / fileDatabaseBatchSize) * fileDatabaseBatchSize` = 1690500
 2. directory name `1690500 / 10_000`, so directory name is `00169`, filename = `1690500 % 10_000`, so filname is `00500.dat`
 3. header contains offsets for blocks 1690500-1690599, block in question will be at entry 53 (0-based)
```
00000190: 549d000000000000 34a0000000000000  T.......4.......
000001a0: 0ca4000000000000 eca6000000000000  ................
000001b0: 94ab000000000000 74ae000000000000  ........t.......
```
the offset at position 53 is 0xA6EC:
```
0000a6e0: .... .... .... .... .... .... 0003 0000              ....
0000a6f0: 0000 0000 < signature data .. .... ....  ................
0000a700: .... .... .... .... .... .... .... ....  ................
0000a710: .... .... .... .... .... .... .... ....  ................
0000a720: .... .... .... .... .... .... .... ....  ................
0000a730: .... .... < public key . .... .... ....  ................
0000a740: .... .... .... .... .... .... .... ....  ................
0000a750: .... ...> 0000 0000 0168 4381 b9cb 1900  .........hC.....
```

0x19cbb9 visible at offset 0xA75C is the block height in hex.

Example 3. Block 1835458 (latest at the moment of writing)
 1. round to batch file id: `(1835458 / fileDatabaseBatchSize) * fileDatabaseBatchSize` = 1835400
 2. directory name is `00183`, file name is `05400.dat`
 3. header contains offsets for blocks 1835400-1835499, block in question will be at entry 58
```
000001c0: c8a6000000000000 a8a9000000000000  ................
000001d0: 88ac000000000000 0000000000000000  ................
000001e0: 0000000000000000 0000000000000000  ................
```

Note: that since this is latest block, all further entries in the offset map are zeroed

Parsing block data is much simpler thanks to catbuffer generated model code.

```python
class BlockDigester:
	OBJECTS_PER_STORAGE_DIRECTORY = 10_000

	def __init__(self, data_path, file_database_batch_size):
		self.data_path = data_path
		self.file_database_batch_size = file_database_batch_size

		# used to cache last opened batch file
		self.batch_file_path = None
		self.batch_file_data = None
		self.block_offsets = None

		self.end = 0

	def get_chain_height(self):
		"""Gets chain height."""

		with open(self.data_path / 'index.dat', 'rb') as input_file:
			data = input_file.read()

		return Height.deserialize(data)

	def parse_offsets(self, ignore_first_zero_offset=False):
		"""There are absolute offsets to blocks at the beginning of the file, parse and store them."""

		buffer = memoryview(self.batch_file_data)

		self.block_offsets = []
		for i in range(self.file_database_batch_size):
			offset = int.from_bytes(buffer[:8], byteorder='little', signed=False)

			# if this is very first batch file it will have 0 as a first entry
			# for any other batch file, 0 means all offsets have been read
			if not offset and not ignore_first_zero_offset and i == 0:
				break

			self.block_offsets.append(offset)
			buffer = buffer[8:]

	def read_batchfile(self, height, force_reread):
		"""Open proper block batch file and parse offsets."""

		group_id = (height // self.file_database_batch_size) * self.file_database_batch_size

		directory = f'{group_id // self.OBJECTS_PER_STORAGE_DIRECTORY:05}'
		name = f'{group_id % self.OBJECTS_PER_STORAGE_DIRECTORY:05}.dat'

		file_path = self.data_path / directory / name

		if self.batch_file_path == file_path and not force_reread:
			return

		with file_path.open(mode='rb', buffering=0) as input_file:
			self.batch_file_data = input_file.read()
			self.batch_file_path = file_path

			self.parse_offsets(group_id == 0)

	def get_block(self, height, force_reread=False):
		"""Returns parsed block at height."""

		self.read_batchfile(height, force_reread)

		entry_in_batch = height % self.file_database_batch_size
		if entry_in_batch >= len(self.block_offsets):
			raise RuntimeError(f'block with given height ({height}) is not present in batch file')

		offset = self.block_offsets[entry_in_batch]
		return BlockFactory.deserialize(self.batch_file_data[offset:])
```

## OTHER

```python
async def create_mosaic_definition_modify(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a transaction that modifies an existing mosaic definition
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'mosaic_definition_transaction',
		'duration': 0,  # number of blocks the mosaic will be active; 0 indicates it will never expire (added to existing value: 0 + 0 = 0)
		'divisibility': 0,  # number of supported decimal places (XOR'd against existing value: 2 ^ 0 = 2)

		# nonce is used as a locally unique identifier for mosaics with a common owner and identifies the mosaic definition to modify
		# mosaic id is derived from the owner's address and the nonce
		'nonce': 123,

		# set of restrictions to apply to the mosaic
		# (XOR'd against existing value: (transferable|restrictable) ^ revokable = transferable|restrictable|revokable)
		# - 'revokable' indicates the mosaic can be revoked by the owner from any account
		'flags': 'revokable'
	})

	# transaction.id field is mosaic id and it is filled automatically after calling transaction_factory.create()

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'mosaic definition transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='mosaic definition transaction')
```
```python
async def create_mosaic_revocation(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic source (it insecurely deterministically generated for the benefit of related tests)
	source_address = facade.network.public_key_to_address(PublicKey(signer_key_pair.public_key.bytes[:-4] + bytes([0, 0, 0, 0])))
	print(f'source: {source_address}')

	# revoke 7 of the custom mosaic from the recipient
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'mosaic_supply_revocation_transaction',
		'source_address': source_address,
		'mosaic': {'mosaic_id': generate_mosaic_id(signer_address, 123), 'amount': 7_00}
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'mosaic revocation transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='mosaic revocation transaction')
```
```python
async def create_mosaic_transfer(facade, signer_key_pair):
	# derive the signer's address
	signer_address = facade.network.public_key_to_address(signer_key_pair.public_key)
	print(f'creating transaction with signer {signer_address}')

	# get the current network time from the network, and set the transaction deadline two hours in the future
	network_time = await get_network_time()
	network_time = network_time.add_hours(2)

	# create a deterministic recipient (it insecurely deterministically generated for the benefit of related tests)
	recipient_address = facade.network.public_key_to_address(PublicKey(signer_key_pair.public_key.bytes[:-4] + bytes([0, 0, 0, 0])))
	print(f'recipient: {recipient_address}')

	# send 10 of the custom mosaic to the recipient
	transaction = facade.transaction_factory.create({
		'signer_public_key': signer_key_pair.public_key,
		'deadline': network_time.timestamp,

		'type': 'transfer_transaction',
		'recipient_address': recipient_address,
		'mosaics': [
			{'mosaic_id': generate_mosaic_id(signer_address, 123), 'amount': 10_00}
		]
	})

	# set the maximum fee that the signer will pay to confirm the transaction; transactions bidding higher fees are generally prioritized
	transaction.fee = Amount(100 * transaction.size)

	# sign the transaction and attach its signature
	signature = facade.sign_transaction(signer_key_pair, transaction)
	facade.transaction_factory.attach_signature(transaction, signature)

	# hash the transaction (this is dependent on the signature)
	transaction_hash = facade.hash_transaction(transaction)
	print(f'mosaic transfer transaction hash {transaction_hash}')

	# finally, construct the over wire payload
	json_payload = facade.transaction_factory.attach_signature(transaction, signature)

	# print the signed transaction, including its signature
	print(transaction)

	# submit the transaction to the network
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP PUT request to a Symbol REST endpoint
		async with session.put(f'{SYMBOL_API_ENDPOINT}/transactions', json=json.loads(json_payload)) as response:
			response_json = await response.json()
			print(f'/transactions: {response_json}')

	# wait for the transaction to be confirmed
	await wait_for_transaction_status(transaction_hash, 'confirmed', transaction_description='mosaic transfer transaction')
```
```python
async def get_network_time():
	async with ClientSession(raise_for_status=True) as session:
		# initiate a HTTP GET request to a Symbol REST endpoint
		async with session.get(f'{SYMBOL_API_ENDPOINT}/node/time') as response:
			# wait for the (JSON) response
			response_json = await response.json()

			# extract the network time from the json
			timestamp = NetworkTimestamp(int(response_json['communicationTimestamps']['receiveTimestamp']))
			print(f'network time: {timestamp} ms')
			return timestamp
```
