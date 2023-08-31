void call(String scriptFilepath) {
	logger.logInfo("Running tests ${scriptFilepath}")
	final String testMode = env.TEST_MODE ?: 'code-coverage'
	final String architecture = helper.resolveBuildArchitecture()
	runScript.withBash("${scriptFilepath} ${testMode} ${architecture}")
}
