def metadata_update_value(old_value, new_value):
	"""Creates a metadata payload for updating old value to new value."""

	if not old_value:
		return new_value

	shorter_length = min(len(old_value), len(new_value))
	longer_length = max(len(old_value), len(new_value))
	is_new_value_shorter = len(old_value) > len(new_value)

	result = bytearray()
	for i in range(shorter_length):
		result.append(old_value[i] ^ new_value[i])

	for i in range(shorter_length, longer_length):
		result.append((old_value if is_new_value_shorter else new_value)[i])

	return bytes(result)
