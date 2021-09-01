from setuptools import setup

with open('README.md', 'r') as readme_file:
    README = readme_file.read()

NAME = 'catbuffer-parser'
VERSION = '1.0.1'

REQUIRES = [
    'catparser>=2.0.0a'
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
    include_package_data=True,
    license='MIT',
    long_description=README,
    long_description_content_type='text/markdown',
    classifiers=[
        'Programming Language :: Python :: 3.6',
        'Development Status :: 7 - Inactive'
    ]
)
