# python 3
"""
Auto-fix include ordering violations reported by checkProjectStructure.py.

Reuses the linter's OWN sorting logic (SortableInclude + the per-directory
rulesets), so the rewritten order is guaranteed to match what the checker
expects, including:
  - own-header-first placement (first_include_check / first_test_include_check)
  - relative-path rewriting (Entry.fix_relative)

Run from the catapult client dir, same as the linter:
    python ../../linters/cpp/fixIncludeOrder.py \
        --dir src --dir sdk --dir tests --dir plugins --dir extensions
Add --dry-run to preview which files would change without writing.
"""

import argparse
import os
import re

import HeaderParser
import validation
from checkProjectStructure import USER_SOURCE_DIRS, Entry, SortableInclude, is_special_include, to_single_root_include
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


def compute_sorted_includes(entry, preprocessor):
	"""Mirror of Entry.check_includes, but returns the include lists instead of reporting."""
	sorted_includes = []
	original_includes = []
	for elem in preprocessor:
		if elem.type != HeaderParser.PpType.INCLUDE:
			continue
		if is_special_include(elem.include):
			continue
		original_includes.append(SortableInclude(elem, entry.ruleset))
		entry.fix_relative(elem)
		sorted_includes.append(SortableInclude(elem, entry.ruleset))

	if not sorted_includes:
		return original_includes, sorted_includes

	full_path = entry.full_path()
	path_elements = re.split(r'[/\\]', full_path)
	sorted_includes.sort()

	if full_path.endswith('.cpp'):
		if 'tests' in path_elements:
			own_header = entry.ruleset.first_test_include_check(sorted_includes, path_elements)
		else:
			own_header = entry.ruleset.first_include_check(sorted_includes, path_elements)

		own_header = to_single_root_include(own_header, path_elements)

		if own_header != sorted_includes[0].include:
			for i, elem in enumerate(sorted_includes):
				if own_header == elem.include:
					sorted_includes.insert(0, sorted_includes.pop(i))
					break

	return original_includes, sorted_includes


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
	original_includes, sorted_includes = compute_sorted_includes(entry, headers.preprocessor)

	if not sorted_includes or original_includes == sorted_includes:
		return False

	# read the physical file preserving each line's original ending
	with open(full_path, 'rb') as input_file:
		raw_lines = input_file.readlines()

	# the slots are the physical line numbers the includes currently occupy, in file order
	slots = sorted(inc.lineno for inc in original_includes)

	# SAFETY: only reorder when the includes form a contiguous block separated solely by blank/comment
	# lines. If any preprocessor directive or code sits between includes (e.g. X-macro #define ... #undef
	# blocks), reordering would change semantics, so skip the file and report it for manual handling.
	slot_set = set(slots)
	for lineno in range(slots[0], slots[-1] + 1):
		if lineno in slot_set:
			continue
		stripped = raw_lines[lineno - 1].decode('utf8').strip()
		if stripped and not stripped.startswith('//'):
			print('SKIP (non-contiguous includes, fix manually):', full_path)
			return False

	for slot_lineno, new_inc in zip(slots, sorted_includes):
		idx = slot_lineno - 1
		old = raw_lines[idx].decode('utf8')
		ending = '\r\n' if old.endswith('\r\n') else ('\n' if old.endswith('\n') else '')
		raw_lines[idx] = (new_inc.line + ending).encode('utf8')

	if dry_run:
		print('would fix', full_path)
		return True

	with open(full_path, 'wb') as output_file:
		output_file.writelines(raw_lines)
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
