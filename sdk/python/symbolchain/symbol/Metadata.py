def metadata_update_value(old_value, new_value):
	"""Creates a metadata payload for updating old value to new value."""

	if not old_value:
		return new_value

	result = []
	for i in range(0, min(len(old_value), len(new_value))):
		result.append(old_value[i] ^ new_value[i])

	result += new_value[len(old_value):len(new_value)]
	return result
