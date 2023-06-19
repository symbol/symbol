pipeline {
	agent {
		label 'ubuntu-small-agent'
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
				stage('cpp - amd64') {
					steps {
						script {
							dispatchBuildCiImageJob('cpp', 'amd64')
						}
					}
				}
				stage('java - amd64') {
					steps {
						script {
							dispatchBuildCiImageJob('java', 'amd64')
						}
					}
				}
				stage('javascript - amd64') {
					steps {
						script {
							dispatchBuildCiImageJob('javascript', 'amd64')
						}
					}
				}
				stage('linter - amd64') {
					steps {
						script {
							dispatchBuildCiImageJob('linter', 'amd64')
						}
					}
				}
				stage('postgres - amd64') {
					steps {
						script {
							dispatchBuildCiImageJob('postgres', 'amd64')
						}
					}
				}
				stage('python - amd64') {
					steps {
						script {
							dispatchBuildCiImageJob('python', 'amd64')
						}
					}
				}

				stage('cpp - arm64') {
					steps {
						script {
							dispatchBuildCiImageJob('cpp', 'arm64')
						}
					}
				}
				stage('java - arm64') {
					steps {
						script {
							dispatchBuildCiImageJob('java', 'arm64')
						}
					}
				}
				stage('javascript - arm64') {
					steps {
						script {
							dispatchBuildCiImageJob('javascript', 'arm64')
						}
					}
				}
				stage('linter - arm64') {
					steps {
						script {
							dispatchBuildCiImageJob('linter', 'arm64')
						}
					}
				}
				stage('postgres - arm64') {
					steps {
						script {
							dispatchBuildCiImageJob('postgres', 'arm64')
						}
					}
				}
				stage('python - arm64') {
					steps {
						script {
							dispatchBuildCiImageJob('python', 'arm64')
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

void dispatchBuildCiImageJob(String ciImage, String architecture) {
	build job: 'build-ci-image', parameters: [
		string(name: 'CI_IMAGE', value: ciImage),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}"),
		string(name: 'ARCHITECTURE', value: architecture),
		booleanParam(
			name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS',
			value: "${!env.SHOULD_PUBLISH_JOB_STATUS || env.SHOULD_PUBLISH_JOB_STATUS.toBoolean()}"
		)
	]
}
