final String shouldPublishFailJobStatusName = 'SHOULD_PUBLISH_FAIL_JOB_STATUS'
final String defaultBranch = 'dev'

pipeline {
	parameters {
		string name: 'JOB_BRANCH_NAME', description: 'branch to trigger jobs', defaultValue: defaultBranch
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
		stage('trigger build jobs') {
			steps {
				script {
					triggerAllJobs(
						env.JOB_BRANCH_NAME ?: defaultBranch,
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

void triggerAllJobs(
		String branchName,
		boolean waitForDownStream,
		String shouldPublishFailJobStatusName,
		boolean shouldPublishFailJobStatusValue) {
	Map<String, String> siblingNameMap = siblingJobNames()
	Map<String, Closure> buildJobs = [:]
	final String platformName = 'PLATFORM'

	siblingNameMap.each { siblingName ->
		String displayName = siblingName.value
		buildJobs["${displayName}"] = {
			stage("${displayName}") {
				String fullJobName = siblingName.key + '/' + branchName.replaceAll('/', '%2F')

				// Platform parameter can vary per project, get the last value
				echo "job name - ${fullJobName}"
				String platform = lastJobParameterValue(fullJobName, platformName)

				build job: "${fullJobName}", parameters: [
					gitParameter(name: 'MANUAL_GIT_BRANCH', value: branchName),
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

Object lastJobParameterValue(String fullJobName, String parameterName) {
	Item job = Jenkins.get().getItemByFullName(fullJobName)
	// groovylint-disable-next-line UnnecessaryGetter
	ParametersAction params = job.getLastBuild().getAction(hudson.model.ParametersAction)
	// groovylint-disable-next-line UnnecessaryGetter
	return params.getParameter(parameterName).getValue()
}
