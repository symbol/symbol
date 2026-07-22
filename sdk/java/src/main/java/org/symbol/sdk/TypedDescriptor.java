package org.symbol.sdk;

import java.util.Map;

/**
 * Type-safe descriptor contract implemented by every generated {@code XxxDescriptor} class; {@link #toMap()} yields the raw descriptor map
 * consumed by the facades and {@link RuleBasedTransactionFactory}.
 */
public interface TypedDescriptor {
	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 *
	 * @return Descriptor map that can be passed to a transaction factory.
	 */
	Map<String, Object> toMap();
}
