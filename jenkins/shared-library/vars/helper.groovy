Boolean isManualBuild(String manualBranch) {
	return null != manualBranch && '' != manualBranch && 'null' != manualBranch
}

String resolveBranchName(String manualBranch) {
	return isManualBuild(manualBranch) ? manualBranch : env.GIT_BRANCH
}

Boolean isPublicBuild() {
	return env.BUILD_CONFIGURATION == 'release-public'
}
