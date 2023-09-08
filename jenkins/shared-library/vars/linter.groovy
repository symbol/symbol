void call(String scriptFilepath) {
	logger.logInfo("Calling linter ${scriptFilepath}")
	runScript.withBash(scriptFilepath)
}
