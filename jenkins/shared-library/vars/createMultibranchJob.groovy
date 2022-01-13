void call(Map jobConfiguration) {
	logger.logInfo("job configuration: ${jobConfiguration}")

	jobDsl scriptText: """
			// Discover branches strategies
			final int EXCLUDE_PULL_REQUESTS_STRATEGY_ID = 1

			// Discover pull requests from origin strategies
			final int USE_CURRENT_SOURCE_STRATEGY_ID = 2

			multibranchPipelineJob(jobName) {
				branchSources {
					branchSource {
						source {
							github {
								// We must set a branch source ID.
								id(repositoryName)
								repoOwner(repositoryOwner)
								repository(repositoryName)
								repositoryUrl(repositoryUrl)
								configuredByUrl(false)

								// Make sure to properly set this.
								credentialsId(credentialsId)

								traits {
									// Depending on your preferences and root pipeline configuration, you can decide to
									// discover branches, pull requests, perhaps even tags.
									gitHubBranchDiscovery {
										strategyId(EXCLUDE_PULL_REQUESTS_STRATEGY_ID)
									}
									gitHubPullRequestDiscovery {
										strategyId(USE_CURRENT_SOURCE_STRATEGY_ID)
									}

									// By default, Jenkins notifies GitHub with a constant context, i.e. a string that
									// identifies the check. We want each individual build result to have its own context so
									// they do not conflict. Requires the github-scm-trait-notification-context-plugin to be
									// installed on the Jenkins instance.
									notificationContextTrait {
										contextLabel(notificationContextLabel)
										typeSuffix(false)
									}
								}
							}
						}

						buildStrategies {
							includeRegionBranchBuildStrategy {
								// Each inclusion uses ant pattern matching, and must be separated by a new line.
								includedRegions(packageIncludePaths)
							}
							excludeRegionBranchBuildStrategy {
								// Each exclusion uses ant pattern matching,and must be separated by a new line.
								excludedRegions(packageExcludePaths)
							}
						}
					}
				}
				factory {
					workflowBranchProjectFactory {
						scriptPath(jenkinsfilePath)
					}
				}
				orphanedItemStrategy {
					discardOldItems {
						daysToKeep(3)
					}
				}
				triggers {
					periodicFolderTrigger {
						interval('7d')
					}
				}
			}
		""", additionalParameters: [
			jobName: jobConfiguration.jobName.toString(),
			repositoryName: jobConfiguration.repositoryName.toString(),
			repositoryOwner: jobConfiguration.repositoryOwner.toString(),
			repositoryUrl: jobConfiguration.repositoryUrl.toString(),
			jenkinsfilePath: jobConfiguration.jenkinsfilePath.toString(),
			packageIncludePaths: jobConfiguration.packageIncludePaths.toString(),
			packageExcludePaths: jobConfiguration.packageExcludePaths.toString(),
			credentialsId: jobConfiguration.credentialsId.toString(),
			notificationContextLabel: "continuous-integration/jenkins/${jobConfiguration.packageFolder}"
	]
}
