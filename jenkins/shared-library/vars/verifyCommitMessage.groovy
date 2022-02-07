void call() {
	runScript('linters/scripts/lint_last_commit.sh')
	logger.logInfo('Verified commit message.')
}
