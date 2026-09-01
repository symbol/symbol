import posixpath
from pathlib import Path, PurePosixPath
from urllib.parse import urlsplit, urlunsplit

import mkdocs_gen_files
from bs4 import BeautifulSoup

nav = mkdocs_gen_files.Nav()

# These Javadoc filenames are useful pages, but their raw stems are poor sidebar labels.
TITLE_OVERRIDES = {
	"package-summary": "PACKAGE SUMMARY",
	"package-tree": "PACKAGE TREE",
}

# Keep package overview pages before the generated class/interface pages in each nav folder.
NAV_ORDER_OVERRIDES = {
	"package-summary": 0,
	"package-tree": 1,
}


def javadoc_path_to_page_dir(path: PurePosixPath) -> PurePosixPath:
	"""
	Returns the MkDocs directory URL path for a source Javadoc HTML path.
	"""
	# MkDocs directory URLs render Foo.md as Foo/ and index.md as the current folder.
	path = path.with_suffix("")
	if path.name == "index":
		return path.parent
	return path


def nav_parts(parts: tuple[str, ...]) -> tuple[str, ...]:
	"""
	Returns the sidebar path for a generated Java reference page.
	"""
	return (*parts[:-1], TITLE_OVERRIDES.get(parts[-1], parts[-1]))


def source_file_sort_key(path: Path) -> tuple[tuple[str, ...], int, str]:
	"""
	Sorts Javadoc source files in the order they should appear in literate-nav.
	"""
	# Sort per source folder first, then apply local package-page priority.
	relative_path = PurePosixPath(path.relative_to(src_dir).as_posix()).with_suffix("")
	return (
		relative_path.parent.parts,
		NAV_ORDER_OVERRIDES.get(relative_path.name, len(NAV_ORDER_OVERRIDES)),
		relative_path.name,
	)


def rewrite_javadoc_link(current_path: PurePosixPath, href: str) -> str:
	"""
	Rewrites internal Javadoc .html links to MkDocs directory-style links.
	"""
	url = urlsplit(href)
	if url.scheme or url.netloc or not url.path.endswith(".html"):
		return href

	# Resolve Javadoc's relative HTML target, then express the equivalent MkDocs page URL
	# relative to the current generated page.
	target_path = PurePosixPath(posixpath.normpath((current_path.parent / url.path).as_posix()))
	current_page_dir = javadoc_path_to_page_dir(current_path)
	target_page_dir = javadoc_path_to_page_dir(target_path)

	if target_page_dir == current_page_dir:
		rewritten_path = "" if url.fragment else "./"
	else:
		rewritten_path = posixpath.relpath(target_page_dir.as_posix(), current_page_dir.as_posix() or ".")
		rewritten_path = f"{rewritten_path}/"

	return urlunsplit(("", "", rewritten_path, url.query, url.fragment))


def transform_javadoc_html(html: str, current_path: PurePosixPath) -> str:
	"""
	Transforms raw Javadoc HTML so it can be embedded inside a MkDocs page.
	"""
	soup = BeautifulSoup(html, "html.parser")

	# Material already provides global navigation, so remove Javadoc's own header chrome.
	for header in soup.select("header.flex-header"):
		header.decompose()

	for link in soup.find_all("a", href=True):
		link["href"] = rewrite_javadoc_link(current_path, link["href"])

	return str(soup)

config = mkdocs_gen_files.config
project_dir = Path(config.config_file_path).resolve().parent

src_dir = config['extra']['symbol']['java-sdk']['javadoc_build_dir']
src_dir = (project_dir / src_dir).resolve()
if not src_dir.is_dir():
	raise FileNotFoundError(
		f"Javadoc build directory does not exist: {src_dir}"
	)

dst_dir = PurePosixPath(config['extra']['symbol']['java-sdk']['output_dir'])
ignored_files = config['extra']['symbol']['java-sdk']['ignore-files']
ignored_folders = config['extra']['symbol']['java-sdk']['ignore-folders']

source_files = sorted((path for path in src_dir.rglob("*") if path.is_file()), key=source_file_sort_key)
for source_file in source_files:
	relative_path = PurePosixPath(source_file.relative_to(src_dir).as_posix())
	if relative_path.suffix.lower() == ".html":
		# Generated files must use docs-relative POSIX paths for mkdocs-gen-files.
		destination = dst_dir / relative_path.with_suffix(".md")

		parts = relative_path.with_suffix('').parts
		if parts[-1] in ignored_files:
			continue
		if any(e in parts for e in ignored_folders):
			continue

		html = transform_javadoc_html(source_file.read_text(encoding="utf-8"), relative_path)
		title = TITLE_OVERRIDES.get(parts[-1])

		with mkdocs_gen_files.open(destination, "w") as output:
			print("---", file=output)
			if title:
				print(f'title: "{title}"', file=output)
			print("hide:", file=output)
			print("  - toc", file=output)
			print("---", file=output)
			print(file=output)
			print('<div class="javadoc-container">', file=output)
			print(html, file=output)
			print("</div>", file=output)

		mkdocs_gen_files.set_edit_path(destination, source_file.as_posix())
		nav[nav_parts(parts)] = relative_path.with_suffix(".md").as_posix()

with mkdocs_gen_files.open('devbook/reference/java/links.md', 'w') as nav_file:
	# Exclude the index file from being indexed by the search plugin
	nav_file.writelines(['---\n', 'search:\n', '    exclude: true\n', '---\n\n'])
	nav_file.writelines(nav.build_literate_nav())
