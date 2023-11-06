void call(String scriptFilepath) {
	logger.logInfo("Running tests ${scriptFilepath}")
	final String testMode = env.TEST_MODE ?: 'code-coverage'
	final String architecture = helper.resolveBuildArchitecture()
	withCredentials([string(credentialsId: 'SEED_ACCOUNT_PRIVATE_KEY_ID', variable: 'SEED_ACCOUNT_PRIVATE_KEY')]) {
		runScript.withBash("${scriptFilepath} ${testMode} ${architecture}")
	}
}
