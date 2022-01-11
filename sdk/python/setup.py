from setuptools import find_packages, setup

with open('README.md', 'rt', encoding='utf8') as readme_file:
    README = readme_file.read()

NAME = 'symbol-sdk-core-python'

with open('version.txt', 'rt', encoding='utf8') as version_file:
    VERSION = version_file.read().strip()

REQUIRES = [
    'cryptography==3.4.6',
    'mnemonic==0.20',
    'Pillow==8.4.0',
    'pysha3==1.0.2',
    'PyYAML==5.4.1',
    'pyzbar==0.1.8',
    'qrcode==7.3.1'
]

setup(
    name=NAME,
    version=VERSION,
    description='Symbol SDK Core',
    author='Symbol Contributors',
    author_email='contributors@symbol.dev',
    url='https://github.com/symbol/sdk-python',
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
