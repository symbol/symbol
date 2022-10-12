pipeline {
	agent any

	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: constants.manualGitBranchName, type: 'PT_BRANCH'
	}

	options {
		ansiColor('css')
		timestamps()
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

		stage('build servers') {
			parallel {
				stage('gcc (metal) [debian]') {
					steps {
						script {
							dispatchBuildJob(constants.gccDebianName, constants.testsMetalName, constants.debianName)
						}
					}
				}

				stage('gcc (westmere)') {
					steps {
						script {
							dispatchBuildJob(constants.gccWestmereName, constants.testsMetalName, constants.ubuntuName)
						}
					}
				}

				stage('gcc (metal) [fedora]') {
					steps {
						script {
							dispatchBuildJob(constants.gccLatestName, constants.testsMetalName, constants.fedoraName)
						}
					}
				}

				stage('clang prior (metal)') {
					steps {
						script {
							dispatchBuildJob(constants.clangPriorName, constants.testsMetalName, constants.ubuntuName)
						}
					}
				}

				stage('clang prior (conan)') {
					steps {
						script {
							dispatchBuildJob(constants.clangPriorName, constants.testsConanName, constants.ubuntuName)
						}
					}
				}

				stage('gcc prior (metal)') {
					steps {
						script {
							dispatchBuildJob(constants.gccPriorName, constants.testsMetalName, constants.ubuntuName)
						}
					}
				}

				stage('gcc prior (conan)') {
					steps {
						script {
							dispatchBuildJob(constants.gccPriorName, constants.testsConanName, constants.ubuntuName)
						}
					}
				}

				stage('msvc prior (metal)') {
					steps {
						script {
							dispatchBuildJob(constants.msvcPriorName, constants.testsMetalName, constants.windowsName)
						}
					}
				}
			}
		}
	}
}

void dispatchBuildJob(String compilerConfiguration, String buildConfiguration, String operatingSystem) {
	build job: 'Symbol/server-pipelines/catapult-client-build-catapult-project', parameters: [
		string(name: 'COMPILER_CONFIGURATION', value: "${compilerConfiguration}"),
		string(name: 'BUILD_CONFIGURATION', value: "${buildConfiguration}"),
		string(name: constants.operatingSystemName, value: "${operatingSystem}"),
		string(name: constants.manualGitBranchName, value: "${params.MANUAL_GIT_BRANCH}")
	]
}
