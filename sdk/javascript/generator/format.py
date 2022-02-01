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
	for idx, entry in enumerate(entries):
		if len(current_line + entry) > LINE_LIMIT - adjust_limit:
			lines.append(current_line)
			current_line = '\t'

		current_line += f'{entry}{", " if idx != len(entries) - 1 else ""}'

	lines.append(current_line)
	lines.append(suffix)

	return '\n'.join(lines)
