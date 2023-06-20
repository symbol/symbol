void call(String buildScriptFilename) {
	String architecture = env.ARCHITECTURE ?: 'amd64'
	runScript("${buildScriptFilename} ${architecture}")
}
