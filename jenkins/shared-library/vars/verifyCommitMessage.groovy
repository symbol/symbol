void call() {
	runScript.withBash('linters/scripts/lint_last_commit.sh')
	logger.logInfo('Verified commit message.')
}
