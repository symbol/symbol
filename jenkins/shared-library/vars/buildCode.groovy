void call(String buildScriptFilename) {
	final String architecture = helper.resolveBuildArchitecture()
	runScript.withBash("${buildScriptFilename} ${architecture}")
}
