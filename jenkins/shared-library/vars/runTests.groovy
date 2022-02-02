void call(String scriptFilepath) {
	logger.logInfo("Running tests ${scriptFilepath}")
	String testMode = env.TEST_MODE ?: 'code-coverage'
	runScript("${scriptFilepath} ${testMode}")
}
