from setuptools import find_packages, setup

with open('README.md', 'r') as readme_file:
    README = readme_file.read()

NAME = 'symbol-sdk-core-python'

with open('version.txt', 'r') as version_file:
    VERSION = version_file.read().strip()

REQUIRES = [
    'catbuffer==1.0.0',
    'cryptography==3.4.6',
    'mnemonic==0.19',
    'Pillow==8.1.1',
    'pysha3==1.0.2',
    'PyYAML==5.4.1',
    'pyzbar==0.1.8',
    'qrcode==6.1'
]

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
        'Programming Language :: Python :: 3.7',
    ]
)
