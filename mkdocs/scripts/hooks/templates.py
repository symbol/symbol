from pathlib import Path

from jinja2 import Environment, FileSystemLoader, select_autoescape


TEMPLATES_DIR = Path(__file__).parent.parent.parent.joinpath("templates", "deploy")
ENVIRONMENT = Environment(
	loader=FileSystemLoader(TEMPLATES_DIR),
	autoescape=select_autoescape(["html", "xml"])
)


def render_template(template_name: str, context: dict) -> str:
	return ENVIRONMENT.get_template(template_name).render(context)
