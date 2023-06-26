void call(String scriptFilepath) {
	logger.logInfo("Running tests ${scriptFilepath}")
	String testMode = env.TEST_MODE ?: 'code-coverage'
	String architecture = env.ARCHITECTURE ?: 'amd64'
	runScript("${scriptFilepath} ${testMode} ${architecture}")
}
