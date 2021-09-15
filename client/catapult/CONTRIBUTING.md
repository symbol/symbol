# Contributing to catapult-client

First off, thank you for considering contributing to catapult-client.

catapult-client is an open source project and we love to receive contributions from
our community — you! There are many ways to contribute, from writing tutorials or blog
posts, improving the documentation, submitting bug reports and feature requests or
writing code which can be incorporated into catapult-client itself.

Following these guidelines helps to communicate that you respect the time of
the developers managing and developing this open source project. In return,
they should reciprocate that respect in addressing your issue, assessing changes,
and helping you finalize your pull requests.

Please, **don't use the issue tracker for support questions**.

## Bug reports

If you think you have found a bug in catapult-client, first make sure that you
are testing against the latest version of catapult-client - your issue may already
have been fixed. If not, search our issues list on GitHub in case a similar
issue has already been opened.

It is very helpful if you can prepare a reproduction of the bug. In other words,
provide a small test case which we can run to confirm your bug. It makes it easier to
find the problem and to fix it.

Please, take in consideration the next template to report your issue:

> **Expected Behaviour**\
> Short and expressive sentence explaining what the code should do.\
> **Current Behaviour**\
> A short sentence enplaning what the code does. \
> **Steps to reproduce**\
> For faster issue detection, we would need a step by step description do reproduce the issue.


Provide as much information as you can.

Open a new issue [here][github-issues].

## Contributing code and documentation changes

If you have a bugfix or new feature that you would like to contribute to catapult-client, please find or open an issue
about it first. Talk about what you would like to do. It may be that somebody is already working on it, or that there
are particular issues that you should know about before implementing the change.

We enjoy working with contributors to get their code accepted. There are many approaches to fixing a problem and it is
important to find the best approach before writing too much code.

### Contributing License Notice

Due to dual licensing of catapult and possible future easing of selected open source licens, when you contribute code,
you'll be required to provide signed waiver of copyrights.

Whether or not you state this explicitly, by submitting any copyrighted material via pull request, email, or other means
you agree to license the material under the project's licenses and warrant that you have the legal authority to do so.

### Fork and clone the repository

You will need to fork the main catapult-client code or documentation repository and clone
it to your local machine. See [github help page](https://help.github.com/articles/fork-a-repo/) for help.

Further instructions for specific projects are given below.

### Submitting your changes

1. Make sure your code meets coding style (file with rules will be provided later).
2. Code without corresponding tests will not be accepted.
3. Run our lint tool `scripts/lint/checkProjectStructure.py --text`
4. Test your change, run tests to make sure that nothing is broken
5. Do **not** use github's *pull request* feature, pull requests will be rejected.
Because of internal review process we require patched generated with `git diff -U100000 base <your-change>`

Then sit back and wait. There will probably be discussion about changes and, if any changes are needed, we would love to work with you to get your changes merged into catapult-client.

*CONTRIBUTING.md is based on [CONTRIBUTING-template.md](https://github.com/nayafia/contributing-template/blob/master/CONTRIBUTING-template.md)* , [elasticsearch/CONTRIBUTING](https://github.com/elastic/elasticsearch/blob/master/CONTRIBUTING.md) and [spark/CONTRIBUTING](https://github.com/apache/spark/blob/master/CONTRIBUTING.md)

[github-issues]: https://github.com/symbol/catapult-client/issues
