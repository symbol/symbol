void call(Map jobConfiguration) {
	logger.logInfo("job configuration: ${jobConfiguration}")

	jobDsl scriptText: """
			// Discover branches strategies
			final int EXCLUDE_PULL_REQUESTS_STRATEGY_ID = 1

			// Discover pull requests from origin strategies
			final int USE_CURRENT_SOURCE_STRATEGY_ID = 2

			multibranchPipelineJob(jobName) {
				displayName(displayName)

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

									cleanBeforeCheckoutTrait {
										extension {
											// Deletes untracked submodules and any other subdirectories which contain .git directories.
											deleteUntrackedNestedRepositories(true)
										}
									}

									submoduleOptionTrait {
										extension {
											// By disabling support for submodules you can still keep using basic git plugin functionality
											// and just have Jenkins to ignore submodules completely as if they didn't exist.
											disableSubmodules(false)

											// Retrieve all submodules recursively (uses '--recursive' option which requires git>=1.6.5)
											recursiveSubmodules(false)

											// Specify a folder containing a repository that will be used by Git as a reference during
											// clone operations.
											reference('')

											// Retrieve the tip of the configured branch in .gitmodules
											// (Uses '--remote' option which requires git>=1.8.2)
											trackingSubmodules(false)

											// Specify a timeout (in minutes) for submodules operations.
											// Default is 10 minutes
											timeout(10)

											// Use credentials from the default remote of the parent project.
											parentCredentials(true)
										}
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
			notificationContextLabel: "continuous-integration/jenkins/${jobConfiguration.packageFolder}",
			displayName: jobConfiguration.displayName.toString()
	]
}
