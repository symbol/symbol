void call() {
	runScript('linters/scripts/run_gitlint.sh')
	logger.logInfo('Verified commit message.')
}
