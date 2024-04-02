void call(Closure body) {
	Map jenkinsfileParams = [:]
	body.resolveStrategy = Closure.DELEGATE_FIRST
	body.delegate = jenkinsfileParams
	body()

	final String shouldPublishFailJobStatusName = 'SHOULD_PUBLISH_FAIL_JOB_STATUS'
	final String defaultBranch = 'dev'

	pipeline {
		parameters {
			gitParameter branchFilter: 'origin/(.*)',
				defaultValue: defaultBranch,
				name: 'MANUAL_GIT_BRANCH',
				type: 'PT_BRANCH',
				selectedValue: 'TOP',
				sortMode: 'ASCENDING',
				useRepository: "${helper.resolveRepoName()}"
			choice name: 'ARCHITECTURE',
				choices: ['arm64', 'amd64'],
				description: 'Computer architecture'
			booleanParam name: shouldPublishFailJobStatusName, description: 'true to publish job status if failed', defaultValue: true
			booleanParam name: 'WAIT_FOR_BUILDS', description: 'true to wait for trigger build to complete', defaultValue: true
		}

		agent {
			label """${
				env.ARCHITECTURE = env.ARCHITECTURE ?: 'arm64'
				return helper.resolveAgentName('ubuntu', env.ARCHITECTURE, 'small')
			}"""
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
							!env.SHOULD_PUBLISH_FAIL_JOB_STATUS || env.SHOULD_PUBLISH_FAIL_JOB_STATUS.toBoolean()
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
		boolean shouldPublishFailJobStatusValue) {
	Map<String, String> displayNameJenkinsfileMap = jobHelper.loadJenkinsfileMap()
	Map<String, String> siblingNameMap = jobHelper.siblingJobNames(displayNameJenkinsfileMap, currentBuild.fullProjectName)
	Map<String, Closure> buildJobs = [:]
	String jobName = jobHelper.resolveJobName(siblingNameMap.keySet().toArray()[0], branchName)

	siblingNameMap.each { siblingName ->
		String displayName = siblingName.value
		buildJobs["${displayName}"] = {
			stage("${displayName}") {
				String fullJobName = siblingName.key + '/' + jobName

				echo "job name - ${fullJobName}"
				Map<String, String> jenkinsfileParameters = jobHelper.readJenkinsFileParameters(displayNameJenkinsfileMap.get(displayName))
				String osValue = jobHelper.resolveOperatingSystem(jenkinsfileParameters.operatingSystem)
				build job: "${fullJobName}", parameters: [
					string(name: 'OPERATING_SYSTEM', value: osValue),
					string(name: 'BUILD_CONFIGURATION', value: 'release-private'),
					string(name: 'TEST_MODE', value: 'code-coverage'),
					string(name: 'ARCHITECTURE', value: params.ARCHITECTURE),
					booleanParam(name: 'SHOULD_PUBLISH_IMAGE', value: false),
					booleanParam(name: shouldPublishFailJobStatusName, value: shouldPublishFailJobStatusValue),
					booleanParam(name: 'SHOULD_RUN_ALL_TEST', value: true)],
					wait: waitForDownStream
			}
		}
	}

	parallel buildJobs
}
