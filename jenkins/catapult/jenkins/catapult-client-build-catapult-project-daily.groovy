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
				stage('gcc latest (conan)') {
					steps {
						script {
							dispatchUbuntuBuildJob(constants.gccLatestName, constants.testsConanName)
						}
					}
				}
				stage('gcc latest (metal)') {
					steps {
						script {
							dispatchUbuntuBuildJob(constants.gccLatestName, constants.testsMetalName)
						}
					}
				}

				stage('clang latest (conan)') {
					steps {
						script {
							dispatchUbuntuBuildJob(constants.clangLatestName, constants.testsConanName)
						}
					}
				}
				stage('clang latest (metal)') {
					steps {
						script {
							dispatchUbuntuBuildJob(constants.clangLatestName, constants.testsMetalName)
						}
					}
				}

				stage('clang ausan') {
					steps {
						script {
							dispatchUbuntuBuildJob(constants.clangAusanName, constants.testsMetalName)
						}
					}
				}
				stage('clang tsan') {
					steps {
						script {
							dispatchUbuntuBuildJob(constants.clangTsanName, constants.testsMetalName)
						}
					}
				}
				stage('clang diagnostics') {
					steps {
						script {
							dispatchUbuntuBuildJob(constants.clangLatestName, 'tests-diagnostics')
						}
					}
				}
				stage('code coverage (gcc latest)') {
					steps {
						script {
							dispatchUbuntuBuildJob('gcc-code-coverage', constants.testsMetalName)
						}
					}
				}
				stage('msvc latest (conan)') {
					steps {
						script {
							dispatchBuildJob(constants.msvcLatestName, constants.testsConanName, constants.windowsName)
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

void dispatchUbuntuBuildJob(String compilerConfiguration, String buildConfiguration) {
	dispatchBuildJob(compilerConfiguration, buildConfiguration, 'ubuntu')
}
