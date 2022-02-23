Boolean isManualBuild(String manualBranch) {
	return null != manualBranch && '' != manualBranch && 'null' != manualBranch
}

String resolveBranchName(String manualBranch) {
	return isManualBuild(manualBranch) ? manualBranch : env.GIT_BRANCH
}

Boolean isPublicBuild(String buildConfiguration) {
	return buildConfiguration == 'release-public'
}

String resolveRepoName() {
	// groovylint-disable-next-line UnnecessaryGetter
	return scm.getUserRemoteConfigs()[0].getUrl().tokenize('/').last()
}

void runInitializeScriptIfPresent() {
	String initFile = 'init.sh'
	if (fileExists(initFile)) {
		logger.logInfo('Running initialize script')
		sh "bash ${initFile}"
	}
}

void configureGitHub() {
	runScript('git config user.name "symbol-bot"')
	// groovylint-disable-next-line GStringExpressionWithinString
	runScript('git config user.email "${GITHUB_EMAIL}"')
}
