import logging
from pathlib import Path
from urllib.parse import urlparse

from mkdocs.config import base

from hooks.templates import render_template

log = logging.getLogger("mkdocs")


def render_root_404_page(config: base.Config) -> str:
	languages = [alternate["lang"] for alternate in config["extra"].get("alternate", [])]
	return render_template("root-404.jinja2", {"supported_languages": languages})


def render_root_index_page(config: base.Config) -> str:
	alternates = config["extra"].get("alternate", [])
	language_urls = {alternate["lang"]: alternate["lang"] for alternate in alternates}
	default_url = alternates[0]["lang"] if alternates else ""
	return render_template(
		"root-index.jinja2",
		{
			"language_urls": language_urls,
			"default_url": default_url,
			"alternates": alternates
		}
	)


def render_cname(config: base.Config) -> str:
	hostname = urlparse(config.site_url).hostname
	if not hostname:
		raise ValueError(f"site_url must include a hostname to generate CNAME: {config.site_url}")
	return f"{hostname}\n"


def write_deploy_root_files(site_root: Path, config: base.Config) -> None:
	(site_root / ".nojekyll").touch()
	(site_root / "CNAME").write_text(render_cname(config), encoding="utf-8")
	(site_root / "index.html").write_text(render_root_index_page(config), encoding="utf-8")
	(site_root / "404.html").write_text(render_root_404_page(config), encoding="utf-8")
	log.info("Custom hook: Wrote root files")
