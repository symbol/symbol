import logging
from pathlib import Path

from mkdocs.config import base

from hooks.templates import render_template

log = logging.getLogger("mkdocs")


def render_redirect_page(target: str) -> str:
	return render_template(
		"redirect.jinja2",
		{"target": target}
	)


def write_redirects(site_root: Path, config: base.Config) -> None:
	redirects = config["extra"]["symbol"].get("redirections", [])
	for redirect in redirects:
		source = redirect["from"].lstrip("/")
		if Path(source).is_absolute() or ".." in Path(source).parts:
			raise ValueError(f"Invalid redirect source path: {redirect['from']}")

		target_path = site_root / source
		target_path.parent.mkdir(parents=True, exist_ok=True)
		target_path.write_text(render_redirect_page(redirect["to"]), encoding="utf-8")
		log.info(f"Custom hook: Wrote redirect {source} -> {redirect['to']}")
