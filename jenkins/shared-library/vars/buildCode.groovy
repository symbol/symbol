void call(String buildScriptFilename) {
	String architecture = helper.resolveBuildArchitecture()
	runScript("${buildScriptFilename} ${architecture}")
}
