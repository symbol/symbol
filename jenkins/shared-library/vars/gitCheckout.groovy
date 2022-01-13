void call(String branch, String credentialsId, String url) {
	logger.logInfo("branch: ${branch}, creds: ${credentialsId}, url:${url}")
	cleanWs()
	git branch: branch, credentialsId: credentialsId, poll: false, url: url
}
