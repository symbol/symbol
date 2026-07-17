"""Field-disposition predicates shared by the model and descriptor struct formatters."""


def is_reserved(field):
	return 'reserved' == field.disposition


def is_bound_size(field):
	return field.extensions.bound_field is not None


def is_const(field):
	return field.is_const


def is_computed(field):
	return hasattr(field.field_type, 'sizeref') and field.field_type.sizeref


def is_inherited_field(field, base_struct):
	"""``True`` when ``field`` is also declared by ``base_struct`` (matched by name)."""
	if not base_struct:
		return False

	return any(field.name == base_field.name for base_field in base_struct.fields)


def filter_size_if_first(fields_iter):
	"""Skip a leading ``size`` field (the transaction-header size is computed, never descriptor-set)."""
	first_field = next(fields_iter, None)
	if first_field is None:
		return

	if 'size' != first_field.name:
		yield first_field

	yield from fields_iter


def descriptor_candidate_fields(fields):
	"""Descriptor-settable fields: non-const, non-reserved, non-bound-size, non-computed, with a leading ``size``
	dropped. The candidate set shared by the model and descriptor struct formatters."""
	candidates = (
		field for field in fields
		if not is_const(field) and not is_reserved(field) and not is_bound_size(field) and not is_computed(field))
	return filter_size_if_first(candidates)
