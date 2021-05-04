import os
import re
from collections import defaultdict


class Rule:  # pylint: disable=too-few-public-methods
    def __init__(self, src, dest):
        self.src = src
        self.dest = dest

    def __repr__(self):
        return 'Rule({}, {})'.format(self.src, self.dest)


class DepsChecker:
    def __init__(self, config_path, errors, verbose=False):
        self.config_path = config_path
        self.errors = errors
        self.verbose = verbose
        self.lines = []
        self.defines = {}
        self.rules = []
        self.read_config()
        self.create_rules()

    def read_config(self):
        own_dir = os.path.dirname(os.path.realpath(__file__))
        config_path = os.path.join(own_dir, self.config_path)
        with open(config_path, 'r') as fin:
            self.parse(fin)

    def parse(self, fin):
        for line in fin:
            # allow rules to be followed by comment
            line = re.sub(r'#.*', '', line.strip())
            line = line.strip()
            if not line or line[0] == '#':
                continue

            rule = re.split(r'->', line)
            if len(rule) == 2:
                self.lines.append(Rule(rule[0].strip(), rule[1].strip()))
            define = re.split(r' = ', line)
            if len(define) == 2:
                deps = filter(None, map(str.strip, define[1].split(' ')))
                self.defines[define[0].strip()] = set(deps)

            if len(rule) != 2 and len(define) != 2:
                raise RuntimeError('could not parse line: {}'.format(line))

    def expand_define(self, expanded, rule_src, rule_dest, level=1):
        if level >= 5:
            raise RuntimeError('define nesting level too deep')

        if rule_src in self.defines:
            for src_dep in self.defines[rule_src]:
                self.expand_define(expanded, src_dep, rule_dest, level + 1)
        elif rule_dest in self.defines:
            for dest_dep in self.defines[rule_dest]:
                self.expand_define(expanded, rule_src, dest_dep, level + 1)
        else:
            expanded.append(Rule(rule_src, rule_dest))

    def process_defines(self):
        expanded_rules = []
        for rule in self.lines:
            self.expand_define(expanded_rules, rule.src, rule.dest)

        return expanded_rules

    @staticmethod
    def is_self_contained(rules, name, deps):
        del name
        return all(map(lambda dep: dep not in rules, deps))

    @staticmethod
    def add_rule(rules, name, dep):
        if dep in rules:
            for subdep in rules[dep]:
                rules[name].add(subdep)

        rules[name].add(dep)

    @staticmethod
    def add_rules(rules, name, deps):
        for dep in deps:
            DepsChecker.add_rule(rules, name, dep)

    def create_rule(self, rule_src, rule_dest):
        src_pattern = '^{}$'.format(rule_src)
        dest_pattern = '^{}$'.format(rule_dest)
        self.rules.append((re.compile(src_pattern), re.compile(dest_pattern)))

    def process_rules(self, expanded_rules):
        transitive_rules = defaultdict(set)
        for rule in expanded_rules:
            transitive_rules[rule.src].add(rule.dest)

        rules = defaultdict(set)
        while transitive_rules:
            current = iter(transitive_rules.items())
            added_rule = False
            while True:
                try:
                    name, deps = next(current)
                except StopIteration:
                    break

                if DepsChecker.is_self_contained(transitive_rules, name, deps):
                    DepsChecker.add_rules(rules, name, deps)
                    del transitive_rules[name]
                    added_rule = True
                    break

            if not added_rule:
                raise RuntimeError('loop in rules detected')

        for rule, rule_set in rules.items():
            for dep in rule_set:
                if self.verbose:
                    print(' + {} -> {}'.format(rule, dep))

                self.create_rule(rule, dep)

    def create_rules(self):
        expanded_rules = self.process_defines()
        self.process_rules(expanded_rules)

    def match(self, path, src_include_dir, dest_include_dir, full_include):
        fixed_dest_dir = dest_include_dir

        # hack: local includes
        # if include contains only signle dir, treat it as a local include and prepend current src dir
        if '/' not in dest_include_dir and dest_include_dir != 'catapult':
            fixed_dest_dir = src_include_dir + '/' + dest_include_dir

        for pattern, allowed in self.rules:
            if pattern.match(src_include_dir) and allowed.match(fixed_dest_dir):
                return True

        dep = '{} -> {}'.format(src_include_dir, dest_include_dir)
        msg = '{:50s} # {} includes {}'.format(dep, path, full_include)
        self.errors.append((src_include_dir, dep, msg))
        return False
