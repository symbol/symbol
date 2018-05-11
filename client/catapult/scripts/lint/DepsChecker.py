from collections import defaultdict
import os
import re

class Rule: # pylint: disable=too-few-public-methods
    def __init__(self, src, dest):
        self.src = src
        self.dest = dest

    def __repr__(self):
        return "Rule({}, {})".format(self.src, self.dest)

class DepsChecker:
    def __init__(self, configPath, errors, verbose=False):
        self.configPath = configPath
        self.errors = errors
        self.verbose = verbose
        self.lines = []
        self.defines = {}
        self.rules = []
        self.readConfig()
        self.createRules()

    def readConfig(self):
        ownDir = os.path.dirname(os.path.realpath(__file__))
        configPath = os.path.join(ownDir, self.configPath)
        with open(configPath, 'r') as fin:
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

    def expandDefine(self, expanded, ruleSrc, ruleDest, level=1):
        if level >= 5:
            raise RuntimeError('define nesting level too deep')

        if ruleSrc in self.defines:
            for srcDep in self.defines[ruleSrc]:
                self.expandDefine(expanded, srcDep, ruleDest, level + 1)
        elif ruleDest in self.defines:
            for destDep in self.defines[ruleDest]:
                self.expandDefine(expanded, ruleSrc, destDep, level + 1)
        else:
            expanded.append(Rule(ruleSrc, ruleDest))

    def processDefines(self):
        expandedRules = []
        for rule in self.lines:
            self.expandDefine(expandedRules, rule.src, rule.dest)

        return expandedRules

    @staticmethod
    def isSelfContained(rules, name, deps):
        del name
        return all(map(lambda dep: dep not in rules, deps))

    @staticmethod
    def addRule(rules, name, dep):
        if dep in rules:
            for subdep in rules[dep]:
                rules[name].add(subdep)

        rules[name].add(dep)

    @staticmethod
    def addRules(rules, name, deps):
        for dep in deps:
            DepsChecker.addRule(rules, name, dep)

    def createRule(self, ruleSrc, ruleDest):
        srcPattern = '^{}$'.format(ruleSrc)
        destPattern = '^{}$'.format(ruleDest)
        self.rules.append((re.compile(srcPattern), re.compile(destPattern)))

    def processRules(self, expandedRules):
        transitiveRules = defaultdict(set)
        for rule in expandedRules:
            transitiveRules[rule.src].add(rule.dest)

        rules = defaultdict(set)
        while transitiveRules:
            current = iter(transitiveRules.items())
            addedRule = False
            while True:
                try:
                    name, deps = next(current)
                except StopIteration:
                    break

                if DepsChecker.isSelfContained(transitiveRules, name, deps):
                    DepsChecker.addRules(rules, name, deps)
                    del transitiveRules[name]
                    addedRule = True
                    break

            if not addedRule:
                raise RuntimeError('loop in rules detected')

        for rule, ruleSet in rules.items():
            for dep in ruleSet:
                if self.verbose:
                    print(' + {} -> {}'.format(rule, dep))

                self.createRule(rule, dep)

    def createRules(self):
        expandedRules = self.processDefines()
        self.processRules(expandedRules)

    def match(self, path, srcIncludeDir, destIncludeDir, fullInclude):
        fixedDestDir = destIncludeDir

        # hack: local includes
        # if include contains only signle dir, treat it as a local include and prepend current src dir
        if '/' not in destIncludeDir and destIncludeDir != 'catapult':
            fixedDestDir = srcIncludeDir + '/' + destIncludeDir

        for pattern, allowed in self.rules:
            if pattern.match(srcIncludeDir) and allowed.match(fixedDestDir):
                return True

        dep = '{} -> {}'.format(srcIncludeDir, destIncludeDir)
        msg = '{:50s} # {} includes {}'.format(dep, path, fullInclude)
        self.errors.append((srcIncludeDir, dep, msg))
        return False
