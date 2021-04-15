from collections import defaultdict
from pathlib import Path
from setuptools import find_packages, setup

with open('README.md', 'r') as readme_file:
    README = readme_file.read()

NAME = 'catbuffer-parser'
VERSION = '1.0.0a3'

REQUIRES = [
]

setup(
    name=NAME,
    version=VERSION,
    description='Symbol Catbuffer Parser',
    author='NEM Group',
    author_email='dev@nem.software',
    url='https://github.com/nemtech/catbuffer-parser',
    keywords=['symbol', 'catbuffer', 'parser', 'catbuffer-parser'],
    install_requires=REQUIRES,
    packages=find_packages(exclude=("test", "test/*")),
    include_package_data=True,
    license='MIT',
    long_description=README,
    long_description_content_type='text/markdown',
    classifiers=[
        'Programming Language :: Python :: 3.6',
    ]
)
