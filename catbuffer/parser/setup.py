from setuptools import find_packages, setup

with open('README.md', 'r') as readme_file:
    README = readme_file.read()

NAME = 'catparser'

with open('version.txt', 'r') as version_file:
    VERSION = version_file.read().strip()

REQUIRES = [
]

setup(
    name=NAME,
    version=VERSION,
    description='Symbol Catbuffer Parser',
    author='NEM Group',
    author_email='dev@nem.software',
    url='https://github.com/symbol/catbuffer-parser',
    keywords=['symbol', 'catbuffer', 'catparser', 'parser', 'catbuffer-parser'],
    install_requires=REQUIRES,
    packages=find_packages(exclude=('tests', 'tests/*')),
    include_package_data=True,
    obsoletes=['catbuffer_parser'],
    license='MIT',
    long_description=README,
    long_description_content_type='text/markdown',
    classifiers=[
        'Programming Language :: Python :: 3.6',
    ]
)
