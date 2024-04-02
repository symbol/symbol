// groovylint-disable-next-line MethodSize
void call(Closure body) {
	Map jenkinsfileParams = [:]
	body.resolveStrategy = Closure.DELEGATE_FIRST
	body.delegate = jenkinsfileParams
	body()

	final String shouldPublishFailJobStatusName = 'SHOULD_PUBLISH_FAIL_JOB_STATUS'
	final String defaultBranch = 'dev'
	String[] projectNames = jenkinsfileParams.projectNames ?: []

	pipeline {
		parameters {
			gitParameter branchFilter: 'origin/(.*)',
				defaultValue: defaultBranch,
				name: 'MANUAL_GIT_BRANCH',
				type: 'PT_BRANCH',
				selectedValue: 'TOP',
				sortMode: 'ASCENDING',
				useRepository: "${helper.resolveRepoName()}"
			choice name: 'PROJECT_NAME',
				choices: jenkinsfileParams.projectNames,
				description: 'Project name'
			choice name: 'ARCHITECTURE',
				choices: ['amd64', 'arm64'],
				description: 'Computer architecture'
			booleanParam name: shouldPublishFailJobStatusName, description: 'true to publish job status if failed', defaultValue: true
			booleanParam name: 'WAIT_FOR_BUILDS', description: 'true to wait for trigger build to complete', defaultValue: true
		}

		agent {
			label """${
				env.ARCHITECTURE = env.ARCHITECTURE ?: 'amd64'
				return helper.resolveAgentName('ubuntu', env.ARCHITECTURE, 'small')
			}"""
		}

		triggers {
			cron('@weekly')
		}

		options {
			ansiColor('css')
			timestamps()
		}

		stages {
			stage('setup projects') {
				when {
					triggeredBy 'UserIdCause'
				}
				steps {
					script {
						// only use the selected project name when triggered by user
						projectNames = [params.PROJECT_NAME]
						println "projectNames: ${projectNames}"
					}
				}
			}
			stage('display environment') {
				steps {
					sh 'printenv'
				}
			}
			stage('checkout') {
				when {
					triggeredBy 'UserIdCause'
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
							projectNames
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
		String[] projectNames) {
	Map<String, String> allJenkinsfileMap = jobHelper.loadJenkinsfileMap()
	Map<String, String> displayNameJenkinsfileMap = allJenkinsfileMap.findAll { namePathMap -> projectNames.contains(namePathMap.key) }
	if (displayNameJenkinsfileMap.isEmpty()) {
		println "No projects found for ${projectNames} in ${allJenkinsfileMap}"
		return
	}

	println "found weekly build projects - ${displayNameJenkinsfileMap}"
	Map<String, String> siblingNameMap = jobHelper.siblingJobNames(displayNameJenkinsfileMap, currentBuild.fullProjectName)
	Map<String, Closure> buildJobs = [:]
	String jobName = jobHelper.resolveJobName(siblingNameMap.keySet().toArray()[0], branchName)

	siblingNameMap.each { siblingName ->
		String displayName = siblingName.value
		Map<String, String> jenkinsfileParameters = jobHelper.readJenkinsFileParameters(displayNameJenkinsfileMap.get(displayName))
		List<String> otherEnvironments = jobHelper.readArrayParameterValue(jenkinsfileParameters.otherEnvironments)

		otherEnvironments.each { environment ->
			String stageName = "${displayName} (${environment})"
			String osValue = environment.split('-')[1]

			buildJobs[stageName] = {
				stage("${stageName}") {
					String fullJobName = siblingName.key + '/' + jobName
					println "job name - ${fullJobName}"

					build job: "${fullJobName}", parameters: [
						string(name: 'OPERATING_SYSTEM', value: osValue),
						string(name: 'BUILD_CONFIGURATION', value: 'release-private'),
						string(name: 'TEST_MODE', value: 'test'),
						string(name: 'ARCHITECTURE', value: params.ARCHITECTURE),
						string(name: 'CI_ENVIRONMENT', value: environment),
						booleanParam(name: 'SHOULD_PUBLISH_IMAGE', value: false),
						booleanParam(name: shouldPublishFailJobStatusName, value: shouldPublishFailJobStatusValue),
						booleanParam(name: 'SHOULD_RUN_ALL_TEST', value: true)],
						wait: waitForDownStream
				}
			}
		}
	}

	parallel buildJobs
}
