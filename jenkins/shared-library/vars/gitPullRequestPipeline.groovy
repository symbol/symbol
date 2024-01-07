// groovylint-disable-next-line MethodSize
void call(Closure body) {
	Map jenkinsfileParams = [:]
	body.resolveStrategy = Closure.DELEGATE_FIRST
	body.delegate = jenkinsfileParams
	body()

	pipeline {
		parameters {
			gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
			booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: true
		}

		agent {
			docker {
				image "${jenkinsfileParams.dockerImageName}"

				// using the same node and the same workspace mounted to the container
				reuseNode true
			}
		}

		options {
			ansiColor('css')
			timestamps()
			timeout(time: 1, unit: 'HOURS')
		}

		stages {
			stage('checkout') {
				when {
					expression { helper.isManualBuild(params.MANUAL_GIT_BRANCH) }
				}
				steps {
					script {
						helper.runStepAndRecordFailure {
							baseBranchName = helper.resolveBranchName(params.MANUAL_GIT_BRANCH)
							sh "git checkout -b ${baseBranchName} origin/${baseBranchName}"
							sh "git reset --hard origin/${baseBranchName}"
						}
					}
				}
			}
			stage('setup environment') {
				steps {
					script {
						helper.runStepAndRecordFailure {
							githubHelper.configureGitHub()
							if (null != jenkinsfileParams.scriptSetupCommand) {
								sh("${jenkinsfileParams.scriptSetupCommand}")
							}
						}
					}
				}
			}
			stage('print env') {
				steps {
					println("Jenkinsfile parameters: ${jenkinsfileParams}")
					sh 'printenv'
				}
			}
			stage('run script') {
				steps {
					script {
						helper.runStepAndRecordFailure {
							sh("${jenkinsfileParams.scriptCommand}")
						}
					}
				}
			}
			stage('create PR') {
				when {
					expression { hasAnyFileChanged(baseBranchName) }
				}
				stages {
					stage('create branch') {
						steps {
							script {
								helper.runStepAndRecordFailure {
									sh("git checkout -b ${jenkinsfileParams.prBranchName}")
								}
							}
						}
					}
					stage('commit and push code') {
						steps {
							script {
								helper.runStepAndRecordFailure {
									githubHelper.executeGitAuthenticatedCommand {
										sh("git push origin ${jenkinsfileParams.prBranchName}")
										final String prTitle = sh(script: 'git log -1 --pretty=%s', returnStdout: true).trim()
										final String prBody = sh(script: 'git log -1 --pretty=%b', returnStdout: true).trim()
										final String ownerName = helper.resolveOrganizationName()
										final String repositoryName = helper.resolveRepositoryName()
										githubHelper.createPullRequestWithReviewers(
											"${GITHUB_ACCESS_TOKEN}",
											ownerName,
											repositoryName,
											jenkinsfileParams.prBranchName,
											baseBranchName,
											prTitle,
											prBody,
											jenkinsfileParams.reviewers ?: []
										)
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
								jenkinsfileParams.discordWebHookUrlId,
								"GitHub job failed for ${currentBuild.fullDisplayName}",
								"GitHub job failed with result of ${currentBuild.currentResult} in"
										+ " stage **${env.FAILED_STAGE_NAME}** with message: **${env.FAILURE_MESSAGE}**.",
								env.BUILD_URL,
								currentBuild.currentResult
						)
					}
				}
			}
		}
	}
}

boolean hasAnyFileChanged(String branchName) {
	return runScript("git diff HEAD..origin/${branchName} --name-only | wc -l", true).trim().toInteger() > 0
}
