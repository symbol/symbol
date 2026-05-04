import sys
import os
import logging
import mkdocs.plugins
from mkdocs.structure import files
from mkdocs.config import Config, base
import shutil
from pathlib import Path
import re
import yaml

log = logging.getLogger('mkdocs')

def build_nav_order(config):
	order = {}
	counter = 0

	def walk(items):
		nonlocal counter

		for item in items:
			if isinstance(item, str):
				order[item] = counter
				counter += 1

			elif isinstance(item, dict):
				for _, value in item.items():
					if isinstance(value, str):
						order[value] = counter
						counter += 1
					elif isinstance(value, list):
						walk(value)

	walk(config.get("nav", []))
	return order

def parse_page_header(text: str) -> tuple[dict, str | None]:
	"""
	Return the YAML frontmatter of a page and its first level 1 header
	"""
	lines = text.splitlines()
	meta = {}
	start = 0

	if lines and lines[0].strip() == "---":
		for i in range(1, len(lines)):
			if lines[i].strip() == "---":
				raw_yaml = "\n".join(lines[1:i])
				meta = yaml.safe_load(raw_yaml) or {}
				start = i + 1
				break

	for line in lines[start:]:
		stripped = line.strip()
		if stripped.startswith("# "):
			return meta, stripped[2:]

	return meta, None

@mkdocs.plugins.event_priority(-50)
def on_files(in_files: files.Files, config: base.Config) -> files.Files:
	"""
	Exclude from processing files we don't care about:
		Doxygen-generated: We only keep filenames starting with configured prefixes.
	Parse frontmatter of developer tutorials to find their level and store it for later.
	"""
	out_files: list[File] = []
	prefixes = tuple(config["extra"]["symbol"]["java-sdk"]["include-prefixes"] + ["links"])
	config['extra']['symbol']['tutorials'] = {}
	nav_order = build_nav_order(config)
	section_order = {}
	for f in in_files:
		if f.src_uri.startswith("devbook/reference/java"):
			if not f.name.startswith(prefixes):
				log.debug(f"Custom hook: Removing {f.name}")
				continue
		out_files.append(f)

		if not f.src_path.startswith("devbook/") or not f.src_path.endswith(".md"):
			continue

		src = Path(config["docs_dir"]) / f.src_path
		if not src.is_file():
			continue
		text = src.read_text(encoding="utf-8")
		meta, title = parse_page_header(text)

		if "tutorial_level" not in meta:
			continue

		section = f.url.split('/')[-3]
		section = section.replace('-', ' ')
		section = section.replace('start', 'getting started')
		section = section.replace('chain', 'chain state')
		level = meta["tutorial_level"]
		if section not in config['extra']['symbol']['tutorials']:
			config['extra']['symbol']['tutorials'][section] = {}
		if level not in config['extra']['symbol']['tutorials'][section]:
			config['extra']['symbol']['tutorials'][section][level] = []
		config['extra']['symbol']['tutorials'][section][level].append({
			"title": meta.get("title") or title,
			"url": '/'.join(f.url.split('/')[1:-1]) + '.md',
			"order": nav_order[f.url[:-1]+'.md']
		})
		section_order[section] = min(section_order.get(section, 999999), nav_order.get(f.src_path, 999999))

	for section in config['extra']['symbol']['tutorials'].values():
		for items in section.values():
			items.sort(key=lambda t: t["order"])

	tutorials = config['extra']['symbol']['tutorials']
	config['extra']['symbol']['tutorials'] = dict(
		sorted(
			tutorials.items(),
			key=lambda item: section_order.get(item[0], 999999),
		)
	)
	return files.Files(out_files)

@mkdocs.plugins.event_priority(50)
def on_pre_build(config: base.Config):
	"""
	Copy the OpenAPI spec file next to its markdown, and load it into the config.
	"""
	spec_path = Path(__file__).parent.parent.parent.joinpath("openapi", "_build").resolve()
	md_path = Path(config.docs_dir).joinpath("devbook", "reference", "rest").resolve()
	spec_fname = 'openapi3.yml'
	shutil.copy2(spec_path / spec_fname, md_path / spec_fname)
	with open(spec_path / spec_fname, 'r', encoding='utf-8') as f:
		config['extra']['symbol']['openapi'] = yaml.safe_load(f)

def page_markdown_js_typedoc(content, page, config, files):
	"""
	Customize markdown for JS API pages. The Typedoc-markdown plugin does not
	support templates so we need this workaround.
	"""
	if not page.url.startswith("devbook/reference/ts"):
		return content

	symbol_name = ''
	parent_name = page.parent.title if page.parent else ''
	def symbol_type_repl(m):
		dict = {"Class":"class", "Function":"method"}
		nonlocal symbol_name
		symbol_name = m.group(2).removesuffix('()')
		if symbol_name == "default":
			nonlocal parent_name
			symbol_name = parent_name
		# Insert manual word breaks in camel-case titles, in case they are very long
		symbol_name_wbr = re.sub(r'([a-z])([A-Z])', r'\1<wbr>\2', symbol_name)
		if m.group(1) not in dict:
			return f'# {m.group(1)}: {symbol_name_wbr}'
		return f'# :simple-javascript: <code class="doc-symbol doc-symbol-heading doc-symbol-{dict[m.group(1)]}"></code> {symbol_name_wbr}'

	# Add object type icon at the header
	content = re.sub(r'^# ([^:]*): ([^\n]*)', symbol_type_repl, content, 1)

	# Add glossary definition to page title
	# Documentation MUST NOT start with # so we can tell it apart from the next markdown heading
	content = re.sub(
		r'^(.*?)\n\n([^#].*?)\n\n',
		rf'\1\n\n<dl class="automatic-reference-term" markdown><dt>js:{symbol_name}</dt><dd>\2</dd></dl>\n\n', content, 1)

	# Add glossary definition to accessors
	# Documentation MUST NOT start with # so we can tell it apart from the next markdown heading
	m = re.search(r'\n## Accessors\n', content)
	if m:
		content = content[:m.start()] + re.sub(
			r"(\n### )([^\n]*?)(\n\n#### Get Signature\n\n```.*?```\n\n)([^#].*?)(\n\n#####)",
			rf'\1\2\3<dl class="automatic-reference-term" markdown><dt>js:{symbol_name}.\2</dt><dd>\4</dd></dl>\5', content[m.start():], flags=re.DOTALL)

	# Add glossary definition to methods
	# Documentation MUST NOT start with # so we can tell it apart from the next markdown heading
	m = re.search(r'\n## Methods\n', content)
	if m:
		content = content[:m.start()] + re.sub(
			r"(\n### )([^(]*?)(\(\)\n\n```.*?```\n\n)([^#].*?)(\n\n####)",
			rf'\1\2\3<dl class="automatic-reference-term" markdown><dt>js:{symbol_name}.\2</dt><dd>\4</dd></dl>\5', content[m.start():], flags=re.DOTALL)

	# Add glossary definition to global functions
	content = re.sub(
		r"(```ts\nfunction .*?)\n\n(.*?)(\n\n## )",
		rf'<dl class="automatic-reference-term" markdown><dt>js:{symbol_name}</dt><dd>\2</dd></dl>\1\2\3', content, flags=re.DOTALL)

	# Add special anchor because the typedoc-md plugin forgot to add it?
	content = re.sub(r'(\n## Constructors)', r'\1<a id="constructor"></a>', content, 1)

	# Replace \c with code tags
	content = re.sub(r'\\c ([^ ]*?) ', r'`\1` ', content)

	# Remove absolute markdown hyperlinks
	content = re.sub(r'\[([^]]*)\]\(/[^)]*\)', r'\1', content)

	# Remove \note tags. They've been mangled when moved from CATS to JS and are barely usable.
	content = re.sub(r'\\note ', '', content)

	return content

def camel_to_snake(name):
	s1 = re.sub(r'(.)([A-Z][a-z]+)', r'\1_\2', name)
	return re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

def page_markdown_dylinks(content, page, config, files):
	"""
	The Dynamic Links (dylinks) parser.
	Turn expressions like <dy:ClassName> and <dy:ClassName.methodName> into links to the reference pages that change
	text and href depending on the selected language.
	Accepts JavaScript.camelCase and reformats to Python.snake_case.
	Settings in mkdocs.base.yml:
	- The dictionary extra.symbol.class-remaps translates from Python names to JS names, because sometimes they're different.
	- The array extra.symbol.global-namespaces lists class names which do not exist in JS and must be removed.
	"""
	langs = ['py', 'js']
	lang_names = ['Python', 'JavaScript']
	class_remaps = config['extra']['symbol']['class-remaps']
	global_namespaces = config['extra']['symbol']['global-namespaces']
	rgroup_id = 999
	def class_formatter(m):
		nonlocal rgroup_id
		r = '<span markdown class="dylink">'
		for ndx, l in enumerate(langs):
			class_name = m.group(1)
			if l == 'py' and class_name in class_remaps:
				class_name = class_remaps[class_name]
			r += f'<input type="radio" name="rGroup{rgroup_id}" id="{lang_names[ndx]}" /><label class="dylink-option" for="{lang_names[ndx]}" markdown><{l}:{class_name}></label>'
		r += '</span>'
		rgroup_id += 1
		return r

	def method_formatter(m):
		nonlocal rgroup_id
		r = '<span markdown class="dylink">'
		for ndx, l in enumerate(langs):
			class_name = m.group(1)
			method_name = m.group(2)
			if l == 'py':
				if class_name in class_remaps:
					class_name = class_remaps[class_name]
				method_name = camel_to_snake(method_name)
			if l == 'js' and class_name in global_namespaces:
				class_name = ""
			r += f'<input type="radio" name="rGroup{rgroup_id}" id="{lang_names[ndx]}" /><label class="dylink-option" for="{lang_names[ndx]}" markdown><{l}:{class_name}{'.' if class_name else ''}{method_name}></label>'
		r += '</span>'
		rgroup_id += 1
		return r

	content = re.sub(r'<dy:([A-Za-z0-9]*)>', class_formatter, content)
	content = re.sub(r'<dy:([A-Za-z0-9]*)\.([A-Za-z0-9_]*)>', method_formatter, content)

	return content

def page_markdown_rest(content, page, config, files):
	def path_formatter(m):
		method = m.group(1)
		path = m.group(2)
		spec = config['extra']['symbol']['openapi']['paths']
		if path not in spec:
			log.warning(f'Page {page.file.src_path} has invalid path {path}')
			return f'**INVALID PATH `{path}`**'
		spec = spec[path]
		if method not in spec:
			log.warning(f'Page {page.file.src_path} has invalid method `{method}` in path {path}')
			return f'**INVALID PATH `{method}:{path}`**'
		spec = spec[method]
		summary = spec['summary']
		r = f'[`{path}`&nbsp;`{method.upper()}`{{.rest-method .rest-method-{method}}}](site:/devbook/reference/rest/symbol#tag/{spec['tags'][0].replace(' ', '_')}/{spec['operationId']} "{summary}")'
		return r

	content = re.sub(r'<(get|put|post):([^>]*)>', path_formatter, content)
	return content

def page_markdown_ws(content, page, config, files):
	content = re.sub(r'(<ws:[^>]*>)', r'\1&nbsp;<code class="rest-method rest-method-ws">WS</code>', content)
	return content

@mkdocs.plugins.event_priority(0)
def on_page_markdown(content, page, config, files):
	content = page_markdown_js_typedoc(content, page, config, files)
	content = page_markdown_dylinks(content, page, config, files)
	content = page_markdown_rest(content, page, config, files)
	content = page_markdown_ws(content, page, config, files)
	return content

class ignoreRESTAnchors(logging.Filter):
	def filter(self, record):
		return not re.search(r'reference/rest/symbol.md.*does not contain an anchor', record.msg)

def on_startup(*args, **kwargs):
	"""
	Add the mkdocs folder to PYTHONPATH, so custom modules like the CATS lexer are found.
	Customize the log level of individual plugins.
	"""
	project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
	if project_root not in sys.path:
		sys.path.insert(0, project_root)
	# Make this noisy plugin shut up a bit
	mkdocs.plugins.get_plugin_logger('mkdocs_site_urls').setLevel(logging.WARNING)
	# Silence messages about missing anchors in links to the REST reference guide because that's a dynamic page
	pagesLog = logging.getLogger('mkdocs.structure.pages')
	pagesLog.addFilter(ignoreRESTAnchors())

def on_nav(nav, config, files):
	"""
	Counts the total number of pages on the site, after autogeneration, and stores it for later use.
	"""
	def count_pages(items):
		count = 0
		for item in items:
			if hasattr(item, 'children') and item.children:
				count += count_pages(item.children)
			else:
				count += 1
		return count

	num_pages = count_pages(nav)
	config['extra']['symbol']['page_count'] = num_pages
	log.info(f"Custom hook: Counted {num_pages} pages")
