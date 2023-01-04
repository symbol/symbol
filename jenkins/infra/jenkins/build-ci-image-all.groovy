pipeline {
	agent {
		label 'ubuntu-agent'
	}

	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		booleanParam name: 'SHOULD_PUBLISH_JOB_STATUS', description: 'true to publish job status', defaultValue: true
	}

	options {
		ansiColor('css')
		timestamps()
	}

	triggers {
		cron('@weekly')
	}

	stages {
		stage('print env') {
			steps {
				echo """
							env.GIT_BRANCH: ${env.GIT_BRANCH}
						 MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}
				"""
			}
		}

		stage('build ci images') {
			parallel {
				stage('cpp') {
					steps {
						script {
							dispatchBuildCiImageJob('cpp')
						}
					}
				}
				stage('java') {
					steps {
						script {
							dispatchBuildCiImageJob('java')
						}
					}
				}
				stage('javascript') {
					steps {
						script {
							dispatchBuildCiImageJob('javascript')
						}
					}
				}
				stage('linter') {
					steps {
						script {
							dispatchBuildCiImageJob('linter')
						}
					}
				}
				stage('postgres') {
					steps {
						script {
							dispatchBuildCiImageJob('postgres')
						}
					}
				}

				stage('python') {
					steps {
						script {
							dispatchBuildCiImageJob('python')
						}
					}
				}
			}
		}
	}
	post {
		success {
			script {
				if (env.SHOULD_PUBLISH_JOB_STATUS?.toBoolean()) {
					helper.sendDiscordNotification(
						':confetti_ball: CI Image All Job Successfully completed',
						'Not much to see here, all is good',
						env.BUILD_URL,
						currentBuild.currentResult
					)
				}
			}
		}
		unsuccessful {
			script {
				if (env.SHOULD_PUBLISH_JOB_STATUS?.toBoolean()) {
					helper.sendDiscordNotification(
						":worried: CI Image All Job Failed for ${currentBuild.fullDisplayName}",
						"At least one job failed for Build#${env.BUILD_NUMBER} which has a result of ${currentBuild.currentResult}.",
						env.BUILD_URL,
						currentBuild.currentResult
					)
				}
			}
		}
	}
}

void dispatchBuildCiImageJob(String ciImage) {
	build job: 'build-ci-image', parameters: [
		string(name: 'CI_IMAGE', value: "${ciImage}"),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}"),
		booleanParam(
			name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS',
			value: "${!env.SHOULD_PUBLISH_JOB_STATUS || env.SHOULD_PUBLISH_JOB_STATUS.toBoolean()}"
		)
	]
}
