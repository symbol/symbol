def indent(text):
	output = ''
	for line in text.splitlines():
		prefix = '\t' if line else ''
		output += f'{prefix}{line}\n'
	return output
