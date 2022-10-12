void call(Closure body) {
	Map params = [:]
	body.resolveStrategy = Closure.DELEGATE_FIRST
	body.delegate = params
	body()

	final String shouldPublishFailJobStatusName = 'SHOULD_PUBLISH_FAIL_JOB_STATUS'
	final String manualGitBranchName = 'MANUAL_GIT_BRANCH'
	final String defaultBranch = 'dev'

	pipeline {
		parameters {
			gitParameter branchFilter: 'origin/(.*)',
				defaultValue: defaultBranch,
				name: manualGitBranchName,
				type: 'PT_BRANCH',
				selectedValue: 'TOP',
				sortMode: 'ASCENDING',
				useRepository: "${helper.resolveRepoName()}"
			booleanParam name: shouldPublishFailJobStatusName, description: 'true to publish job status if failed', defaultValue: true
			booleanParam name: 'WAIT_FOR_BUILDS', description: 'true to wait for trigger build to complete', defaultValue: true
		}

		agent {
			label 'ubuntu-agent'
		}

		triggers {
			cron('@midnight')
		}

		options {
			ansiColor('css')
			timestamps()
		}

		stages {
			stage('display environment') {
				steps {
					sh 'printenv'
				}
			}
			stage('checkout') {
				when {
					expression { helper.isManualBuild(env.MANUAL_GIT_BRANCH) }
				}
				steps {
					script {
						sh "git checkout ${helper.resolveBranchName(env.MANUAL_GIT_BRANCH)}"
						sh "git reset --hard origin/${helper.resolveBranchName(env.MANUAL_GIT_BRANCH)}"
					}
				}
			}
			stage('trigger build jobs') {
				steps {
					script {
						triggerAllJobs(
							env.MANUAL_GIT_BRANCH ?: defaultBranch,
							!env.WAIT_FOR_BUILDS || env.WAIT_FOR_BUILDS.toBoolean(),
							shouldPublishFailJobStatusName,
							!env.SHOULD_PUBLISH_FAIL_JOB_STATUS || env.SHOULD_PUBLISH_FAIL_JOB_STATUS.toBoolean(),
							manualGitBranchName
						)
					}
				}
			}
		}
		post {
			unsuccessful {
				script {
					if (null != env.SHOULD_PUBLISH_FAIL_JOB_STATUS && env.SHOULD_PUBLISH_FAIL_JOB_STATUS.toBoolean()) {
						helper.sendDiscordNotification(
							"Jenkins Job Failed for ${currentBuild.fullDisplayName}",
							"At least one job failed for Build#${env.BUILD_NUMBER} which has a result of ${currentBuild.currentResult}.",
							env.BUILD_URL,
							currentBuild.currentResult
						)
					}
				}
			}
		}
	}
}

void triggerAllJobs(
		String branchName,
		boolean waitForDownStream,
		String shouldPublishFailJobStatusName,
		boolean shouldPublishFailJobStatusValue,
		String manualGitBranchName) {
	Map<String, String> siblingNameMap = siblingJobNames()
	Map<String, String> displayNameJenkinsfileMap = jenkinsfileMap()
	Map<String, Closure> buildJobs = [:]
	final String platformName = 'PLATFORM'
	String jobName = resolveJobName(siblingNameMap.keySet().toArray()[0], branchName)

	siblingNameMap.each { siblingName ->
		String displayName = siblingName.value
		buildJobs["${displayName}"] = {
			stage("${displayName}") {
				String fullJobName = siblingName.key + '/' + jobName

				// Platform parameter can vary per project, get the last value
				echo "job name - ${fullJobName}"
				String platform = jobParameterValueFromJenkinsfile(displayNameJenkinsfileMap.get(displayName), platformName.toLowerCase())
				build job: "${fullJobName}", parameters: [
					gitParameter(name: manualGitBranchName, value: branchName),
					string(name: platformName, value: platform ?: 'ubuntu'),
					string(name: 'BUILD_CONFIGURATION', value: 'release-private'),
					string(name: 'TEST_MODE', value: 'code-coverage'),
					booleanParam(name: 'SHOULD_PUBLISH_IMAGE', value: false),
					booleanParam(name: shouldPublishFailJobStatusName, value: shouldPublishFailJobStatusValue)],
					wait: waitForDownStream
			}
		}
	}

	parallel buildJobs
}

Map<String, String> siblingJobNames() {
	Item project = Jenkins.get().getItemByFullName(currentBuild.fullProjectName)
	List<Item> siblingItems = project.parent.items

	Map<String, String> targets = [:]
	for (Item item in siblingItems) {
		if (org.jenkinsci.plugins.workflow.multibranch.WorkflowMultiBranchProject != item.getClass() || item.fullName == project.fullName) {
			continue
		}

		targets.put(item.fullName, item.displayName)
	}

	return targets
}

String jobParameterValueFromJenkinsfile(String jenkinsfilePath, String parameterName) {
	String value = sh(
		script: "sed -rn \'s/${parameterName} = (.*)\$/\\1/p\' ${jenkinsfilePath}/Jenkinsfile",
		returnStdout: true
	)
	value = value.replaceAll("(\\[|\\]|')", '').trim()
	echo "Found ${parameterName} = ${value}"
	return value
}

Map <String, String> jenkinsfileMap() {
	Object buildConfiguration = yamlHelper.readYamlFromFile(helper.resolveBuildConfigurationFile())
	Map<String, String> displayNameJenkinsfileMap = [:]

	buildConfiguration.builds.each { build -> displayNameJenkinsfileMap.put(build.name, build.path) }
	return displayNameJenkinsfileMap
}

String resolveJobName(String jobFolder, String branchName) {
	final String separator = '/'
	String encodeJobName = branchName.replaceAll(separator, '%2F')
	String fullJobName = jobFolder + separator + encodeJobName
	Item job = Jenkins.get().getItemByFullName(fullJobName)
	if (job?.isBuildable()) {
		return encodeJobName
	}

	// find the PR that matches the branch
	List<Item> jobs = Jenkins.get().getItemByFullName(jobFolder).items
	for (Item item in jobs) {
		if (item.name.startsWith('PR-') && item.isBuildable()) {
			String changeBranch = item.lastBuild.environment.get('CHANGE_BRANCH', '')
			if (changeBranch == branchName) {
				echo "found PR: ${item.name}"
				return item.name
			}
		}
	}

	throw new IllegalStateException("Multibranch job folder ${jobFolder} does not contain a job with branch ${branchName}")
}
