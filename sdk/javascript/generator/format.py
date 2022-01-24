LINE_LIMIT = 140


def indent(text):
	output = ''
	for line in text.splitlines():
		prefix = '\t' if line else ''
		output += f'{prefix}{line}\n'
	return output


def wrap_lines(entries, prefix, suffix, adjust_limit=0):
	lines = [prefix]

	current_line = '\t'
	for i, entry in enumerate(entries):
		if len(current_line + entry) > LINE_LIMIT - adjust_limit:
			lines.append(current_line)
			current_line = '\t'
		elif 0 != i:
			current_line += ' '

		current_line += str(entry)
		if i != len(entries) - 1:
			current_line += ','

	lines.append(current_line)
	lines.append(suffix)

	return '\n'.join(lines)
