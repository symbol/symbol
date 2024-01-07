import groovy.json.JsonOutput

boolean isGitHubRepositoryPublic(String orgName, String repoName) {
	try {
		final URL url = "https://api.github.com/repos/${orgName}/${repoName}".toURL()
		final Object repo = yamlHelper.readYamlFromText(url.text)

		return repo.name == repoName && repo.visibility == 'public'
	} catch (FileNotFoundException exception) {
		println "Repository ${orgName}/${repoName} not found - ${exception}"
		return false
	}
}

String executeGithubApiRequest(String command) {
	final String response = runScript(command, true)
	if (response.contains('message')) {
		println "GitHub API request failed: ${response}"
		throw new IllegalArgumentException(response)
	}

	return response
}

// groovylint-disable-next-line FactoryMethodName
Object createPullRequest(
	String token,
	String ownerName,
	String repositoryName,
	String prBranchName,
	String baseBranchName,
	String title,
	String body
) {
	final String jsonBody = JsonOutput.toJson(body)
	final String pullRequestCommand = """
		curl -L \\
			-X POST \\
			-H "Accept: application/vnd.github+json" \\
			-H "Authorization: Bearer ${token}" \\
			-H "X-GitHub-Api-Version: 2022-11-28" \\
			https://api.github.com/repos/${ownerName}/${repositoryName}/pulls \\
			-d '{"title":"${title}","body":${jsonBody},"head":"${prBranchName}","base":"${baseBranchName}"}'
	"""

	final String pullRequestResponse = executeGithubApiRequest(pullRequestCommand)
	final Object pullRequest = yamlHelper.readYamlFromText(pullRequestResponse)
	println "Pull request created: ${pullRequest.number}"
	return pullRequest
}

Object requestReviewersForPullRequest(String token, String ownerName, String repositoryName, int pullRequestNumber, List reviewers) {
	final String reviewersCommand = """
		curl -L \\
			-X POST \\
			-H "Accept: application/vnd.github+json" \\
			-H "Authorization: Bearer ${token}" \\
			-H "X-GitHub-Api-Version: 2022-11-28" \\
			https://api.github.com/repos/${ownerName}/${repositoryName}/pulls/${pullRequestNumber}/requested_reviewers \\
			-d '{"reviewers": ["${reviewers.join('", "')}"]}'
	"""

	String response = executeGithubApiRequest(reviewersCommand)
	println "Reviewers requested: ${response}"
	return yamlHelper.readYamlFromText(response)
}

// groovylint-disable-next-line FactoryMethodName
Object createPullRequestWithReviewers(
	String token,
	String ownerName,
	String repositoryName,
	String prBranchName,
	String baseBranchName,
	String title,
	String body,
	List reviewers
) {
	Object pullRequest = createPullRequest(token, ownerName, repositoryName, prBranchName, baseBranchName, title, body)
	if (!reviewers.isEmpty()) {
		final int pullRequestNumber = pullRequest.number.toInteger()
		pullRequest = requestReviewersForPullRequest(token, ownerName, repositoryName, pullRequestNumber, reviewers)
	}

	return pullRequest
}

void configureGitHub() {
	runScript('git config user.name "symbol-bot"')
	runScript('git config user.email "jenkins@symbol.dev"')
}

void executeGitAuthenticatedCommand(Closure command) {
	withCredentials([usernamePassword(credentialsId: helper.resolveGitHubCredentialsId(),
			usernameVariable: 'GITHUB_APP',
			passwordVariable: 'GITHUB_ACCESS_TOKEN')]) {
		final String ownerName = helper.resolveOrganizationName()
		final String replaceUrl = "https://${GITHUB_APP}:${GITHUB_ACCESS_TOKEN}@github.com/${ownerName}.insteadOf" +
			" 'https://github.com/${ownerName}'"
		sh("git config url.${replaceUrl}")
		configureGitHub()
		command()
	}
}
