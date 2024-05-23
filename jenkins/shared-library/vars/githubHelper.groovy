import groovy.json.JsonOutput

boolean isGitHubRepositoryPublic(String orgName, String repoName) {
	try {
		executeGitAuthenticatedCommand {
			final Object repo = getRepositoryInfo("${GITHUB_TOKEN}", orgName, repoName)
			return repo.name == repoName && repo.visibility == 'public'
		}
	} catch (FileNotFoundException exception) {
		println "Repository ${orgName}/${repoName} not found - ${exception}"
		return false
	}
}

Object getRepositoryInfo(String token, String ownerName, String repositoryName) {
	final String getRepoCommand = new StringBuilder().append('curl -L -H "Accept: application/vnd.github+json"')
		.append(""" -H "Authorization: Bearer ${token}" """)
		.append("""-H "X-GitHub-Api-Version: 2022-11-28"  https://api.github.com/repos/${ownerName}/${repositoryName}""")

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
	final String pullRequestCommand = new StringBuilder().append('curl -L -X POST -H "Accept: application/vnd.github+json" ')
		.append("""-H "Authorization: Bearer ${token}" """)
		.append("""-H "X-GitHub-Api-Version: 2022-11-28" https://api.github.com/repos/${ownerName}/${repositoryName}/pulls """)
		.append("""-d '{"title":"${title}","body":${jsonBody},"head":"${prBranchName}","base":"${baseBranchName}"}'""")

	final String pullRequestResponse = executeGithubApiRequest(pullRequestCommand)
	final Object pullRequest = yamlHelper.readYamlFromText(pullRequestResponse)
	println "Pull request created: ${pullRequest.number}"
	return pullRequest
}

Object requestReviewersForPullRequest(String token, String ownerName, String repositoryName, int pullRequestNumber, List reviewers) {
	final String reviewersCommand = new StringBuilder().append('curl -L -X POST -H "Accept: application/vnd.github+json" ')
		.append("""-H "Authorization: Bearer ${token}" """)
		.append('-H "X-GitHub-Api-Version: 2022-11-28" ')
		.append("""https://api.github.com/repos/${ownerName}/${repositoryName}/pulls/${pullRequestNumber}/requested_reviewers """)
		.append("""-d '{"reviewers": ["${reviewers.join('", "')}"]}'""")

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
			usernameVariable: 'GITHUB_USER',
			passwordVariable: 'GITHUB_TOKEN')]) {
		final String replaceUrl = "https://${GITHUB_USER}:${GITHUB_TOKEN}@github.com/.insteadOf https://github.com/"

		configureGitHub()
		runScript("git config url.${replaceUrl}")
		command()
	}
}
