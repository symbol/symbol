import java.time.LocalDateTime

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
	return scm.userRemoteConfigs[0].url.tokenize('/').last()
}

void runInitializeScriptIfPresent() {
	String initFile = 'init.sh'
	if (fileExists(initFile)) {
		logger.logInfo('Running initialize script')
		runScript("bash ${initFile}")
		runScript('git submodule update --remote')
	}
}

String resolveBuildConfigurationFile() {
	return '.github/buildConfiguration.yaml'
}

String resolveAgentName(String os) {
	return 'windows' == os ? 'windows-xlarge-agent' : 'ubuntu-xlarge-agent'
}

String resolveAgentName(String os, String architecture, String size) {
	if ('macos' == os) {
		return 'macos-agent'
	}

	return 'windows' == os ? "windows-${size}-${architecture}-agent" : "ubuntu-${size}-${architecture}-agent"
}

void sendDiscordNotification(String title, String description, String url, String result, String footer = '') {
	withCredentials([string(credentialsId: 'DISCORD_WEB_HOOK_URL_ID', variable: 'WEB_HOOK_URL')]) {
		discordSend description: description, footer: footer, link: url, result: result, title: title, webhookURL: "${env.WEB_HOOK_URL}"
	}
}

void runStepAndRecordFailure(Closure body) {
	try {
		body()
		// groovylint-disable-next-line CatchException
	} catch (Exception exception) {
		echo "Stage ${env.STAGE_NAME} failed with exception: ${exception}"
		env.FAILURE_MESSAGE = exception.message ?: exception
		env.FAILED_STAGE_NAME = env.STAGE_NAME
		throw exception
	}
}

String determineArchitecture() {
	String architecture = LocalDateTime.now().dayOfMonth % 2 == 0 ? 'amd64' : 'arm64'
	println "Architecture: ${architecture}"
	return architecture
}

boolean isAmd64Architecture(String architecture) {
	return 'amd64' == architecture
}

String resolveBuildArchitecture() {
	return env.ARCHITECTURE ?: 'amd64'
}

String resolveOrganizationName() {
	return env.GIT_URL.tokenize('/')[2]
}

String resolveRepositoryName() {
	return env.GIT_URL.tokenize('/').last().split('\\.')[0]
}

String resolveUrlBase(String url) {
	URL urlObject = new URL(url)
	return "${urlObject.protocol}://${urlObject.host}"
}

String resolveUrlHostName(String url) {
	return new URL(url).host
}

boolean tryRunCommand(Closure command) {
	try {
		command()
		return true
		// groovylint-disable-next-line CatchException
	} catch (_) {
		return false
	}
}

String resolveWorkspacePath(String os) {
	return 'windows' == os ? 'C:\\Users\\Administrator\\jenkins\\workspace\\' : '/home/ubuntu/jenkins/workspace/'
}

String resolveGitHubCredentialsId() {
	return scm.userRemoteConfigs[0].credentialsId
}
