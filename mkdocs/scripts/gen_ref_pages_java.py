import re
from pathlib import Path, PurePosixPath

import mkdocs_gen_files

nav = mkdocs_gen_files.Nav()

config = mkdocs_gen_files.config
project_dir = Path(config.config_file_path).resolve().parent

src_dir = config['extra']['symbol']['java-sdk']['javadoc_build_dir']
src_dir = (project_dir / src_dir).resolve()
if not src_dir.is_dir():
	raise FileNotFoundError(
		f"Javadoc build directory does not exist: {src_dir}"
	)

dst_dir = config['extra']['symbol']['java-sdk']['output_dir']
ignored_files = config['extra']['symbol']['java-sdk']['ignore-files']
ignored_folders = config['extra']['symbol']['java-sdk']['ignore-folders']

source_files = sorted(path for path in src_dir.rglob("*") if path.is_file())
for source_file in source_files:
	relative_path = PurePosixPath(source_file.relative_to(src_dir).as_posix())
	if relative_path.suffix.lower() == ".html":
		destination = dst_dir / relative_path.with_suffix(".md")

		parts = relative_path.with_suffix('').parts
		if parts[-1] in ignored_files:
			continue
		if any(e in parts for e in ignored_folders):
			continue

		html = source_file.read_text(encoding="utf-8")

		with mkdocs_gen_files.open(destination, "w") as output:
			print("---", file=output)
			print("hide:", file=output)
			print("  - toc", file=output)
			print("---", file=output)
			print(file=output)
			print('<div class="javadoc-container">', file=output)
			print(html, file=output)
			print("</div>", file=output)
			print("<style>.javadoc-container header.flex-header { display: none; }</style>", file=output)

		mkdocs_gen_files.set_edit_path(destination, source_file.as_posix())
		nav[parts] = relative_path.with_suffix(".md").as_posix()

with mkdocs_gen_files.open('devbook/reference/java/links.md', 'w') as nav_file:
	# Exclude the index file from being indexed by the search plugin
	nav_file.writelines(['---\n', 'search:\n', '    exclude: true\n', '---\n\n'])
	nav_file.writelines(nav.build_literate_nav())
