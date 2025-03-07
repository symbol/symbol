import logging
import mkdocs.plugins
from mkdocs.structure import files
from mkdocs.config import Config, base
import shutil
from pathlib import Path
import re

log = logging.getLogger('mkdocs')

@mkdocs.plugins.event_priority(-50)
def on_files(in_files: files.Files, config: base.Config) -> files.Files:
    """
    Exclude from processing files we don't care about.
    Doxygen-generated: We only keep filenames starting with configured prefixes.
    Typedoc-generated: Remove index README files.
    """
    out_files: list[File] = []
    prefixes = tuple(config["extra"]["symbol"]["java-sdk"]["include-prefixes"] + ["links"])
    for f in in_files:
        if f.src_uri.startswith("devbook/reference/java"):
            if not f.name.startswith(prefixes):
                log.debug(f"Custom hook: Removing {f.name}")
                continue
        if f.src_uri.startswith("devbook/reference/ts"):
            if f.src_uri.endswith("README.md"):
                log.debug(f"Custom hook: Removing {f.name}")
                continue
        out_files.append(f)

    return files.Files(out_files)

@mkdocs.plugins.event_priority(50)
def on_pre_build(config: base.Config):
    """
    Copy OpenAPI spec files next to their markdown.
    """
    spec_path = Path(__file__).parent.parent.parent.joinpath("openapi").resolve()
    md_path = Path(config.docs_dir).joinpath("devbook", "reference", "rest").resolve()
    for f in ['openapi-symbol.yml']:
        shutil.copyfile(spec_path / f, md_path / f)

@mkdocs.plugins.event_priority(0)
def on_page_markdown(content, page, config, files):
    """
    Customize markdown for TS API pages. The Typedoc-markdown plugin does not
    support templates so we need this workaround.
    """
    if not page.url.startswith("devbook/reference/ts"):
        return content

    symbol_name = ''
    def symbol_type_repl(m):
        dict = {"Class":"class", "Function":"method"}
        nonlocal symbol_name
        symbol_name = m.group(2)
        if m.group(1) not in dict:
            return f'# {m.group(1)}: {m.group(2)}'
        return f'# <code class="doc-symbol doc-symbol-heading doc-symbol-{dict[m.group(1)]}"></code> {m.group(2)}'

    # Add object type icon at the header
    content = re.sub(r'^# ([^:]*): ([^\n]*)', symbol_type_repl, content, 1)

    # Add glossary definition to page title
    # Documentation MUST NOT start with # so we can tell it apart from the next markdown heading
    content = re.sub(
        r'^(.*?)\n\n([^#].*?)\n\n',
        rf'\1\n\n<dl class="automatic-reference-term" markdown><dt>TS:{symbol_name}</dt><dd>\2</dd></dl>\n\n', content, 1)

    # Add glossary definition to methods
    # Documentation MUST NOT start with # so we can tell it apart from the next markdown heading
    m = re.search(r'\n## Methods\n', content)
    if m:
        content = content[:m.start()] + re.sub(
            r"(\n### )([^(]*?)(\(\)\n\n```.*?```\n\n)([^#].*?)(\n\n####)",
            rf'\1\2\3<dl class="automatic-reference-term" markdown><dt>TS:{symbol_name}.\2</dt><dd>\4</dd></dl>\5', content[m.start():], flags=re.DOTALL)

    return content
