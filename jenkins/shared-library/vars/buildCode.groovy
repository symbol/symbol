void call(String buildScriptFilename) {
	final String architecture = helper.resolveBuildArchitecture()
	runScript("${buildScriptFilename} ${architecture}")
}
