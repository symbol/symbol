"""Generate the Python code reference pages."""

from pathlib import Path

import mkdocs_gen_files

nav = mkdocs_gen_files.Nav()

root = Path(__file__).parent.parent.parent
src = root / "sdk/python/symbolchain"
paths = sorted(src.rglob("*.py"))

for path in paths:
    module_path = path.relative_to(src.parent).with_suffix("")
    doc_path = path.relative_to(src).with_suffix(".md")
    full_doc_path = Path("en/devbook/reference/py", doc_path)

    parts = tuple(module_path.parts)

    if parts[-1] == "__init__":
        parts = parts[:-1]
    elif parts[-1] == "__main__":
        continue

    nav[parts] = doc_path.as_posix()

    with mkdocs_gen_files.open(full_doc_path, "w") as fd:
        identifier = ".".join(parts)
        print("::: " + identifier, file=fd)

    mkdocs_gen_files.set_edit_path(full_doc_path, path.relative_to(root))

with mkdocs_gen_files.open("en/devbook/reference/py/navigation.md", "w") as nav_file:
    nav_file.writelines(nav.build_literate_nav())
