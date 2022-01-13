void call(String job_name, String package_folder, String package_include_paths, String package_exclude_paths, String jenkinsfile_path,
          String repository_url, String repository_owner, String repository_name, String credentials_id) {

    jobDsl scriptText: '''
            // Discover branches strategies
            final int EXCLUDE_PULL_REQUESTS_STRATEGY_ID = 1
        
            // Discover pull requests from origin strategies
            final int USE_CURRENT_SOURCE_STRATEGY_ID = 2
        
            multibranchPipelineJob(job_name) {
                branchSources {
                    branchSource {
                        source {
                            github {
                                // We must set a branch source ID.
                                id(repository_name)
                                repoOwner(repository_owner)
                                repository(repository_name)
                                repositoryUrl(repository_url)
                                configuredByUrl(false)
    
                                // Make sure to properly set this.
                                credentialsId(credentials_id)
    
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
                                        contextLabel("continuous-integration/jenkins/${package_folder}")
                                        typeSuffix(false)
                                    }
                                }
                            }
                        }
    
                        buildStrategies {
                            includeRegionBranchBuildStrategy {
                                // Each inclusion uses ant pattern matching, and must be separated by a new line.
                                includedRegions(package_include_paths)
                            }
                            excludeRegionBranchBuildStrategy {
                                // Each exclusion uses ant pattern matching,and must be separated by a new line.
                                excludedRegions(package_exclude_paths)
                            } 
                        }
                    }
                }
                factory {
                    workflowBranchProjectFactory {
                        scriptPath(jenkinsfile_path)
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
        ''', additionalParameters: [
            job_name         : job_name,
            repository_name  : repository_name,
            repository_owner : repository_owner,
            repository_url   : repository_url,
            jenkinsfile_path : jenkinsfile_path,
            package_include_paths : package_include_paths,
            package_exclude_paths : package_exclude_paths,
            credentials_id   : credentials_id,
            package_folder   : package_folder
    ]
}