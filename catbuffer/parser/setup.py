from setuptools import find_packages, setup

with open('README.md', 'rt', encoding='utf8') as readme_file:
    README = readme_file.read()

NAME = 'catparser'

with open('version.txt', 'rt', encoding='utf8') as version_file:
    VERSION = version_file.read().strip()

REQUIRES = [
]

setup(
    name=NAME,
    version=VERSION,
    description='Symbol Catbuffer Parser',
    author='Symbol Contributors',
    author_email='contributors@symbol.dev',
    url='https://github.com/symbol/catbuffer-parser',
    keywords=['symbol', 'catbuffer', 'catparser', 'parser', 'catbuffer-parser'],
    install_requires=REQUIRES,
    packages=find_packages(exclude=('tests', 'tests/*')),
    data_files=[('grammar', ['catparser/grammar/catbuffer.lark'])],
    include_package_data=True,
    obsoletes=['catbuffer_parser'],
    license='MIT',
    long_description=README,
    long_description_content_type='text/markdown',
    classifiers=[
        'Programming Language :: Python :: 3.6',
    ]
)
