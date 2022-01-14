from ..CryptoTypes import Hash256, PublicKey
from ..TypeParserBuilder import TypeParserBuilder
from .Network import Address


def extend_type_parsing_rules(symbol_type_converter, base_type_parsing_rules=None):
	builder = TypeParserBuilder('sc', symbol_type_converter, base_type_parsing_rules)

	# note, some of those below could be automatically generated,
	# but right now symbol.yml contains also state-related objects
	flag_parsers = {
		name: builder.create_flags_parser(name) for name in ['MosaicFlags', 'AccountRestrictionFlags']
	}
	enum_parsers = {
		name: builder.create_enum_parser(name) for name in [
			'TransactionType', 'AliasAction', 'LinkAction', 'LockHashAlgorithm',
			'MosaicRestrictionType', 'MosaicSupplyChangeAction', 'NamespaceRegistrationType'
		]
	}
	struct_parsers = {
		f'struct:{name}': builder.create_struct_parser(name, extend_type_parsing_rules) for name in ['UnresolvedMosaic']
	}

	sdk_type_mapping = {
		'UnresolvedAddress': Address,
		'Address': Address,
		'Hash256': Hash256,

		'PublicKey': PublicKey,
		'VotingPublicKey': PublicKey,
	}
	sdk_type_wrappers = {
		name: builder.create_sdk_wrapper(typename) for name, typename in sdk_type_mapping.items()
	}

	default_rules = {**flag_parsers, **enum_parsers, **struct_parsers, **sdk_type_wrappers}
	builder.add_base_value_rules(default_rules)

	final_rules = default_rules if not base_type_parsing_rules else {**default_rules, **base_type_parsing_rules}

	for name in ['UnresolvedMosaicId', 'TransactionType', 'UnresolvedAddress']:
		final_rules[f'array[{name}]'] = builder.create_object_array_parser(f'{name}', final_rules)

	for name in ['UnresolvedMosaic']:
		final_rules[f'array[{name}]'] = builder.create_object_array_parser(f'struct:{name}', final_rules)

	return final_rules
