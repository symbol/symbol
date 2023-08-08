void call(String scriptFilepath) {
	logger.logInfo("Running tests ${scriptFilepath}")
	String testMode = env.TEST_MODE ?: 'code-coverage'
	String architecture = helper.resolveBuildArchitecture()
	runScript("${scriptFilepath} ${testMode} ${architecture}")
}
