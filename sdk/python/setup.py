from setuptools import find_packages, setup

with open('README.md', 'r') as readme_file:
    README = readme_file.read()

NAME = 'symbol-sdk-core-python'
VERSION = '0.2.0.dev1'

REQUIRES = []

setup(
    name=NAME,
    version=VERSION,
    description='Symbol SDK Core',
    author='NEM Group',
    author_email='dev@nem.software',
    url='https://github.com/nemtech/symbol-sdk-core-python',
    keywords=['symbol', 'sdk', 'sdk-core', 'Symbol SDK core'],
    install_requires=REQUIRES,
    package_dir={'': '.'},
    packages=find_packages('.'),
    include_package_data=True,
    license='MIT',
    long_description=README,
    long_description_content_type='text/markdown',
    classifiers=[
        'Programming Language :: Python :: 3.6',
    ]
)
