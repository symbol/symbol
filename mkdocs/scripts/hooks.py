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

@mkdocs.plugins.event_priority(-50)
def on_files(in_files: files.Files, config: base.Config) -> files.Files:
	"""
	Exclude from processing files we don't care about.
	Doxygen-generated: We only keep filenames starting with configured prefixes.
	"""
	out_files: list[File] = []
	prefixes = tuple(config["extra"]["symbol"]["java-sdk"]["include-prefixes"] + ["links"])
	for f in in_files:
		if f.src_uri.startswith("devbook/reference/java"):
			if not f.name.startswith(prefixes):
				log.debug(f"Custom hook: Removing {f.name}")
				continue
		out_files.append(f)

	return files.Files(out_files)

@mkdocs.plugins.event_priority(50)
def on_pre_build(config: base.Config):
	"""
	Copy the OpenAPI spec file next to its markdown, and load it into the config.
	"""
	spec_path = Path(__file__).parent.parent.parent.joinpath("openapi").resolve()
	md_path = Path(config.docs_dir).joinpath("devbook", "reference", "rest").resolve()
	spec_fname = 'openapi-symbol.yml'
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
		symbol_name = m.group(2)
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

	# Add special anchor because the typedoc-md plugin forgot to add it?
	content = re.sub(r'(\n## Constructors)', r'\1<a id="constructor"></a>', content, 1)

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
	"""
	langs = ['py', 'js']
	lang_names = ['Python', 'JavaScript']
	class_remaps = config['extra']['symbol']['class-remaps']
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
			r += f'<input type="radio" name="rGroup{rgroup_id}" id="{lang_names[ndx]}" /><label class="dylink-option" for="{lang_names[ndx]}" markdown><{l}:{class_name}.{method_name}></label>'
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
		r = f'[`{path}`&nbsp;`{method.upper()}`{{.rest-method .rest-method-{method}}}](site:/devbook/reference/rest/symbol#operations-{spec['tags'][0].replace(' ', '_')}-{spec['operationId']} "{summary}")'
		return r

	content = re.sub(r'<(get|put|post):([^>]*)>', path_formatter, content)
	return content

@mkdocs.plugins.event_priority(0)
def on_page_markdown(content, page, config, files):
	content = page_markdown_js_typedoc(content, page, config, files)
	content = page_markdown_dylinks(content, page, config, files)
	content = page_markdown_rest(content, page, config, files)
	return content

def on_startup(*args, **kwargs):
	"""
	Add the mkdocs folder to PYTHONPATH, so custom modules like the CATS lexer are found.
	"""
	project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
	if project_root not in sys.path:
		sys.path.insert(0, project_root)

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
