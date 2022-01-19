void call(String scriptFilepath) {
	logger.logInfo("Calling linter ${scriptFilepath}")
	runScript("bash ${scriptFilepath}")
}
