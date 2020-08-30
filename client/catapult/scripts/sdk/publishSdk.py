# python 3
import argparse
from fileinput import FileInput
import os
import re
import shutil


# region file system utils

def find_headers(directory):
    return [f for f in os.listdir(directory) if os.path.isfile(os.path.join(directory, f)) and f.endswith('.h')]


def find_subdirectories(directory):
    return [f for f in os.listdir(directory) if os.path.isdir(os.path.join(directory, f))]


def require_existence(path):
    if not os.path.exists(path):
        raise Exception('{} does not exist'.format(path))


def require_non_existence(path):
    if os.path.exists(path):
        raise Exception('{} already exists'.format(path))


def force_empty(directory):
    if os.path.exists(directory):
        shutil.rmtree(directory)

    os.makedirs(directory)

# endregion


# region post processing

def fix_includes(path):
    for line in FileInput(path, inplace=True):
        line = re.sub(r'#include "src/', '#include "catapult/', line.rstrip())
        line = re.sub(r'#include ".*/src/', '#include "catapult/', line.rstrip())
        line = re.sub(r'#include "plugins/.*/src/', '#include "catapult/', line.rstrip())
        print(line)

# endregion


class Publisher:
    def __init__(self, root_directory, publish_directory):
        force_empty(publish_directory)

        self.root_directory = os.path.abspath(root_directory)
        self.root_catapult_directory = os.path.join(root_directory, 'src', 'catapult')
        self.publish_directory = os.path.abspath(publish_directory)
        self.publish_catapult_directory = os.path.join(publish_directory, 'catapult')

        self.verbose = False
        self.headers = []
        self.source_directories = []

        self.register_source_directory(self.root_catapult_directory)

    def set_verbose(self, verbose):
        self.verbose = verbose

    def publish_component(self, component):
        # ensure the source directory exists
        # (notice that self.root_catapult_directory is the source_directory of all components)
        component_directory = os.path.join(self.root_catapult_directory, component)
        require_existence(component_directory)

        # publish all headers
        self.publish_headers(component_directory, component)

    def publish_components(self, plugin_directory, components):
        # ensure the source directory exists and register it as a source_directory
        self.register_source_directory(plugin_directory)

        for component in components:
            component_directory = os.path.join(plugin_directory, component)
            if not os.path.exists(component_directory):
                continue

            # publish all headers
            self.publish_headers(component_directory, component)

    def publish_extension(self, extension, components):
        plugin_directory = os.path.join(self.root_directory, 'extensions', extension, 'src')
        self.publish_components(plugin_directory, components)

    def publish_plugin(self, group, plugin, components):
        plugin_directory = os.path.join(self.root_directory, 'plugins', group, plugin, 'src')
        self.publish_components(plugin_directory, components)

    def publish_merged_file(self, filename):
        merged_path = os.path.join(self.publish_catapult_directory, filename)
        merged_file = open(merged_path, 'w')
        self.headers.append(os.path.join('catapult', filename))

        for source_directory in self.source_directories:
            component_file = os.path.join(source_directory, filename)
            if not os.path.exists(component_file):
                continue

            for line in FileInput(component_file):
                merged_file.write(line)

    def publish_sdk_extensions(self, component):
        # ensure the source directory exists
        source_component_directory = os.path.join(self.root_directory, 'sdk', 'src', component)
        require_existence(source_component_directory)

        # publish all headers
        self.publish_headers(source_component_directory, component)

    def flush_master_header(self, exclusions=None):
        master_header_path = os.path.join(self.publish_catapult_directory, 'catapult.h')
        master_header_file = open(master_header_path, 'w')

        master_header_file.writelines(['#pragma once', '\n'])
        for header in self.headers:
            path_parts = os.path.split(header)
            if None is exclusions or path_parts[-1] not in exclusions:
                master_header_file.writelines(['#include "{}"'.format(header), '\n'])

    def register_source_directory(self, source_directory):
        require_existence(source_directory)
        self.source_directories.append(source_directory)

    def publish_headers(self, source_directory, component):
        # ensure the destination directory exists
        publish_component_directory = os.path.join(self.publish_catapult_directory, component)
        os.makedirs(publish_component_directory, exist_ok=True)

        # find and publish all headers
        self.log('***** processing {} ****'.format(source_directory))
        for header in find_headers(source_directory):
            self.publish_header(source_directory, component, header)

        # search first-level subdirectories
        for subdirectory in find_subdirectories(source_directory):
            self.log('***** processing {}/{} ****'.format(source_directory, subdirectory))

            # ensure the destination directory exists
            os.makedirs(os.path.join(publish_component_directory, subdirectory), exist_ok=True)

            # find and publish all headers
            for header in find_headers(os.path.join(source_directory, subdirectory)):
                self.publish_header(source_directory, component, os.path.join(subdirectory, header))

    def publish_header(self, source_directory, component, header):
        self.headers.append(os.path.join('catapult', component, header))

        source_file = os.path.join(source_directory, header)
        publish_file = os.path.join(self.publish_catapult_directory, component, header)

        require_non_existence(publish_file)
        shutil.copyfile(source_file, publish_file)

        fix_includes(publish_file)

    def log(self, *args):
        if self.verbose:
            print(*args)


def publish_all():
    parser = argparse.ArgumentParser(description='publishes the catapult sdk')
    parser.add_argument('-r', '--root', help='the root directory', required=True)
    parser.add_argument('-p', '--publish', help='the publish directory', required=True)
    parser.add_argument('-v', '--verbose', help='output verbose information', action='store_true')

    args = parser.parse_args()

    publisher = Publisher(args.root, args.publish)
    publisher.set_verbose(args.verbose)

    for component in ['api', 'config', 'crypto', 'crypto_voting', 'io', 'ionet', 'model', 'net', 'state', 'thread', 'utils', 'version']:
        publisher.publish_component(component)

    transactions = [
        'account_link', 'aggregate', 'lock_hash', 'lock_secret', 'metadata', 'mosaic', 'multisig', 'namespace',
        'restriction_account', 'restriction_mosaic', 'transfer'
    ]
    for transaction in transactions:
        publisher.publish_plugin('txes', transaction, ['model', 'plugins', 'state'])

    publisher.publish_plugin('coresystem', '', ['model', 'plugins'])

    for service in ['hashcache']:
        publisher.publish_plugin('services', service, ['state'])

    for extension in ['finalization']:
        publisher.publish_extension(extension, ['api', 'io', 'model'])

    for extension in ['nodediscovery']:
        publisher.publish_extension(extension, ['api'])

    for filename in ['constants', 'exceptions', 'functions', 'plugins', 'preprocessor', 'types']:
        publisher.publish_merged_file(filename + '.h')

    for component in ['builders', 'extensions']:
        publisher.publish_sdk_extensions(component)

    publisher.flush_master_header(['MacroBasedEnum.h'])


publish_all()
