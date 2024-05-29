import groovy.json.JsonOutput

boolean isGitHubRepositoryPublic(String orgName, String repoName) {
	try {
		withGitHubToken {
			final Object repo = getRepositoryInfo("${GITHUB_TOKEN}", orgName, repoName)
			return repo.name == repoName && repo.visibility == 'public'
		}
	} catch (FileNotFoundException exception) {
		println "Repository ${orgName}/${repoName} not found - ${exception}"
		return false
	}
}

// groovylint-disable-next-line FactoryMethodName
String buildCurlCommand(String token, String url, String data = null, Boolean post = false) {
	return [
		'curl -L',
		'--fail',
		post ? '-X POST' : '',
		'-H \"Accept: application/vnd.github+json\"',
		"-H \"Authorization: Bearer ${token}\"",
		'-H \"X-GitHub-Api-Version: 2022-11-28\"',
		url,
		data ? "-d \'${data}\'" : ''
	].join(' ')
}

Object getRepositoryInfo(String token, String ownerName, String repositoryName) {
	final String getRepoCommand = buildCurlCommand(token, "https://api.github.com/repos/${ownerName}/${repositoryName}")
	final String getRepoResponse = executeGithubApiRequest(getRepoCommand)
	return yamlHelper.readYamlFromText(getRepoResponse)
}

String executeGithubApiRequest(String command) {
	String results = ''
	helper.withTempDir {
		String file = "./${System.currentTimeMillis()}"
		runScript("${command} -s -o ${file}")
		results = readFile(file).trim()
	}

	return results
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
	final String pullRequestCommand = buildCurlCommand(
		token,
		"https://api.github.com/repos/${ownerName}/${repositoryName}/pulls",
		"{\"title\":\"${title}\",\"body\":${jsonBody},\"head\":\"${prBranchName}\",\"base\":\"${baseBranchName}\"}",
		true
	)
	final String pullRequestResponse = executeGithubApiRequest(pullRequestCommand)
	final Object pullRequest = yamlHelper.readYamlFromText(pullRequestResponse)
	println "Pull request created: ${pullRequest.number}"
	return pullRequest
}

Object requestReviewersForPullRequest(String token, String ownerName, String repositoryName, int pullRequestNumber, List reviewers) {
	final String reviewersCommand = buildCurlCommand(
		token,
		"https://api.github.com/repos/${ownerName}/${repositoryName}/pulls/${pullRequestNumber}/requested_reviewers",
		"{\"reviewers\": [\"${reviewers.join('\", \"')}\"]}",
		true
	)
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

void withGitHubToken(Closure closure) {
	withCredentials([usernamePassword(credentialsId: helper.resolveGitHubCredentialsId(),
			usernameVariable: 'GITHUB_USER',
			passwordVariable: 'GITHUB_TOKEN')]) {
		closure()
	}
}

void executeGitAuthenticatedCommand(Closure command) {
	withGitHubToken {
		final String ownerName = helper.resolveOrganizationName()
		final String replaceUrl = 'https://$GITHUB_USER:$GITHUB_TOKEN@github.com/' +
			"${ownerName}/.insteadOf https://github.com/${ownerName}/"

		configureGitHub()
		runScript("git config url.${replaceUrl}")
		command()
	}
}
