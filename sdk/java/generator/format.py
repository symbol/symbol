"""Source-code formatting helpers (line wrapping + tab indent) for the Java generator."""

LINE_LIMIT = 140


def indent(text):
	"""Indent every non-empty line of `text` with a single tab and ensure trailing newlines."""
	output = ''
	for line in text.splitlines():
		prefix = '\t' if line else ''
		output += f'{prefix}{line}\n'

	return output


def _statement_end(lines, index):
	depth = 0
	while index < len(lines):
		stripped = lines[index].strip()
		depth += stripped.count('(') - stripped.count(')')
		if 0 >= depth and stripped.endswith(';'):
			return index

		index += 1

	return index - 1


def _condition_end(lines, index):
	depth = 0
	while index < len(lines):
		depth += lines[index].count('(') - lines[index].count(')')
		if 0 >= depth:
			return index

		index += 1

	return index - 1


def _block_end(lines, index):
	depth = 0
	while index < len(lines):
		depth += lines[index].count('{') - lines[index].count('}')
		if 0 >= depth:
			return index

		index += 1

	return index - 1


def _if_construct_end(lines, index):
	while True:
		header_end = _condition_end(lines, index)
		if lines[header_end].rstrip().endswith('{'):
			end = _block_end(lines, header_end)
		else:
			end = _statement_end(lines, header_end + 1)

		closing = lines[end].strip()
		if 'else' in closing and closing.startswith('}') and not closing.endswith(';'):
			# `} else {` / `} else if (...) {` — chain continues on the closing line itself
			index = end
			continue

		if end + 1 < len(lines) and lines[end + 1].strip().startswith('else'):
			index = end + 1
			continue

		return end


def separate_if_blocks(source):
	"""Insert a blank line after every if/else construct that is directly followed by another statement."""
	lines = source.split('\n')
	insert_after = set()
	for index, line in enumerate(lines):
		stripped = line.strip()
		if not (stripped.startswith('if (') or stripped.startswith('if(')):
			continue

		end = _if_construct_end(lines, index)
		following = lines[end + 1].strip() if end + 1 < len(lines) else ''
		if following and not following.startswith(('}', ')', 'case ', 'default')):
			insert_after.add(end)

	output = []
	for index, line in enumerate(lines):
		output.append(line)
		if index in insert_after:
			output.append('')

	return '\n'.join(output)


def _javadoc(lines, prefix='\t'):
	output = f'{prefix}/**\n'
	for line in lines:
		output += f'{prefix} *\n' if not line else f'{prefix} * {line}\n'

	output += f'{prefix} */\n'
	return output


def _method(doc_lines, signature, body):
	doc = _javadoc(doc_lines)
	return f'{doc}{signature} {{\n{body}\t}}\n'
