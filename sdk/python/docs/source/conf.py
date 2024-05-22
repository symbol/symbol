# pylint: disable=invalid-name

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import configparser
import sys
from datetime import date
from pathlib import Path

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information


def read_unquoted_value(config, key):
	return config[key].strip('\'')


def read_first_name(config, key):
	return config[key].strip('[\']').split(' <')[0]


def load_project_information():
	# pylint: disable=redefined-outer-name

	config = configparser.ConfigParser()
	config.read(Path('../../pyproject.toml'))
	poetry_config = config['tool.poetry']

	author = read_first_name(poetry_config, 'authors')
	description = read_unquoted_value(poetry_config, 'description')
	version = read_unquoted_value(poetry_config, 'version')

	project = f'{description} (Python)'
	project_copyright = f'{date.today().year}, {author}'
	release = version

	return (project, project_copyright, author, release)


(project, project_copyright, author, release) = load_project_information()

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ['sphinx.ext.autodoc', 'sphinx_rtd_theme']

templates_path = ['_templates']
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = []

# -- Path setup --------------------------------------------------------------

sys.path.insert(0, str(Path('../..').resolve()))
