import logging
import mkdocs.plugins
from mkdocs.structure import files
from mkdocs.config import Config, base
import shutil
from pathlib import Path

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
