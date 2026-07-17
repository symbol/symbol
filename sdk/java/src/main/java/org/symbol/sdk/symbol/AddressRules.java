package org.symbol.sdk.symbol;

/**
 * Descriptor parsing rules for Symbol address fields, shared by the transaction/block/receipt factories and kept off the public
 * {@link Address} type. The resolved/unresolved split mirrors the model: only an unresolved address may carry a namespace alias.
 */
final class AddressRules {

	private AddressRules() {
	}

	/**
	 * Parses a descriptor value into a resolved model address, rejecting a namespace alias — used as the {@code Address} rule for fields
	 * that cannot hold one (e.g. a block beneficiary or an alias transaction's target).
	 *
	 * @param value Descriptor value (Address, base32 string, byte array, or SDK ByteArray).
	 * @return Resolved model address.
	 */
	static org.symbol.sdk.symbol.models.Address resolved(final Object value) {
		final Address address = Address.parse(value);
		if (address.isAlias())
			throw new IllegalArgumentException("resolved address field cannot be a namespace alias: " + address);

		return new org.symbol.sdk.symbol.models.Address(address.bytes());
	}
}
