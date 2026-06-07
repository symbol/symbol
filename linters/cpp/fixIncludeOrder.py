# python 3
"""
Auto-fix include ordering violations reported by checkProjectStructure.py.

Reuses the linter's OWN sorting logic (Entry.compute_sorted_includes), so the
rewritten order is guaranteed to match what the checker expects: own-header-first
placement and relative-path rewriting.

Run from the catapult client dir, same as the linter. For example:
python ../../linters/cpp/fixIncludeOrder.py --dir src --dir sdk --dir tests
Pass --dry-run to preview which files would change without writing.
"""

import argparse
import os
import re

import HeaderParser
import validation
from checkProjectStructure import USER_SOURCE_DIRS, Entry
from exclusions import SKIP_FILES
from Rules import RULE_ID_TO_CLASS_MAP


def ruleset_for_path(full_path):
	parts = re.split(r'[/\\]', full_path)
	top = parts[0]
	if top == 'internal' and len(parts) > 1 and parts[1] == 'tools':
		top = 'internal/tools'
	if top not in USER_SOURCE_DIRS:
		return None
	return RULE_ID_TO_CLASS_MAP[USER_SOURCE_DIRS[top]]


def includes_are_contiguous(raw_lines, slots):
	# only reorder when the includes form a contiguous block separated solely by blank/comment lines.
	# if any preprocessor directive or code sits between includes (e.g. X-macro #define ... #undef blocks),
	# reordering would change semantics, so the file is skipped and reported for manual handling.
	slot_set = set(slots)
	for lineno in range(slots[0], slots[-1] + 1):
		if lineno in slot_set:
			continue
		stripped = raw_lines[lineno - 1].decode('utf8').strip()
		if stripped and not stripped.startswith('//'):
			return False
	return True


def write_sorted_includes(full_path, raw_lines, slots, sorted_includes):
	# rewrite each include slot (preserving its original line ending) with the include sorted into that position
	for slot_lineno, new_inc in zip(slots, sorted_includes):
		idx = slot_lineno - 1
		old = raw_lines[idx].decode('utf8')
		ending = '\r\n' if old.endswith('\r\n') else ('\n' if old.endswith('\n') else '')
		raw_lines[idx] = (new_inc.line + ending).encode('utf8')

	with open(full_path, 'wb') as output_file:
		output_file.writelines(raw_lines)


def fix_file(full_path, dry_run):
	for skip_file in SKIP_FILES:
		if skip_file.match(full_path):
			return False

	ruleset = ruleset_for_path(full_path)
	if ruleset is None:
		return False

	path, filename = os.path.split(full_path)
	entry = Entry(path, filename, ruleset)

	# parse with a no-op error reporter; we only want the preprocessor token stream
	headers = HeaderParser.HeaderParser(lambda *a, **k: None, full_path, validation.create_validators())
	original_includes, sorted_includes, _ = entry.compute_sorted_includes(headers.preprocessor)

	if not sorted_includes or original_includes == sorted_includes:
		return False

	with open(full_path, 'rb') as input_file:
		raw_lines = input_file.readlines()

	# the slots are the physical line numbers the includes currently occupy, in file order
	slots = sorted(inc.lineno for inc in original_includes)
	if not includes_are_contiguous(raw_lines, slots):
		print('SKIP (non-contiguous includes, fix manually):', full_path)
		return False

	if dry_run:
		print('would fix', full_path)
		return True

	write_sorted_includes(full_path, raw_lines, slots, sorted_includes)
	print('fixed', full_path)
	return True


def main():
	parser = argparse.ArgumentParser(description='auto-fix catapult include ordering')
	parser.add_argument('-d', '--dir', action='append', required=True, help='source directory to scan (repeatable)')
	parser.add_argument('--dry-run', action='store_true', help='report files that would change without writing')
	args = parser.parse_args()

	changed = 0
	for source_dir in args.dir:
		if not os.path.exists(source_dir):
			print('x skipping missing directory', source_dir)
			continue
		for root, _, files in os.walk(source_dir):
			for filename in files:
				if not filename.endswith('.h') and not filename.endswith('.cpp'):
					continue
				if fix_file(os.path.join(root, filename), args.dry_run):
					changed += 1

	print('---')
	print('{} file(s) {}'.format(changed, 'would change' if args.dry_run else 'fixed'))


if __name__ == '__main__':
	main()
