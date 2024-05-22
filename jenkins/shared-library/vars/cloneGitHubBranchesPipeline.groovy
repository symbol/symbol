// groovylint-disable-next-line MethodSize
void call(Closure body) {
	Map jenkinsfileParams = [:]
	body.resolveStrategy = Closure.DELEGATE_FIRST
	body.delegate = jenkinsfileParams
	body()

	pipeline {
		parameters {
			booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: true
		}

		agent {
			label "${helper.resolveAgentName('ubuntu', 'arm64', 'small')}"
		}

		triggers {
			cron('@weekly')
		}

		options {
			ansiColor('css')
			timestamps()
		}

		stages {
			stage('clone repo') {
				steps {
					script {
						helper.runStepAndRecordFailure {
							sh "git clone ${jenkinsfileParams.sourceRepoUrl} repo"
							helper.runStepRelativeToPackageRoot 'repo', {
								jenkinsfileParams.branches.each { branch ->
									sh "git checkout ${branch}"
								}
							}
						}
					}
				}
			}
			stage('add remote destination url') {
				steps {
					script {
						helper.runStepAndRecordFailure {
							helper.runStepRelativeToPackageRoot 'repo', {
								sh "git remote set-url origin ${jenkinsfileParams.destRepoUrl}"
								sh 'git remote -v'
							}
						}
					}
				}
			}
			stage('push branches to destination repo') {
				steps {
					script {
						helper.runStepAndRecordFailure {
							helper.runStepRelativeToPackageRoot 'repo', {
								githubHelper.executeGitAuthenticatedCommand {
									sh 'git fetch --all'
									jenkinsfileParams.branches.each { branch ->
										sh "git push -f origin ${branch}:${branch}"
									}
								}
							}
						}
					}
				}
			}
		}
		post {
			unsuccessful {
				script {
					if (env.SHOULD_PUBLISH_FAIL_JOB_STATUS?.toBoolean()) {
						helper.sendDiscordNotification(
							"Repo job clone failed for ${currentBuild.fullDisplayName}",
							"Job for ${jenkinsfileParams.sourceRepoUrl} -> ${jenkinsfileParams.destRepoUrl} repo has result of " +
								"${currentBuild.currentResult} in stage **${env.FAILED_STAGE_NAME}** with message: **${env.FAILURE_MESSAGE}**.",
							env.BUILD_URL,
							currentBuild.currentResult
						)
					}
				}
			}
		}
	}
}
