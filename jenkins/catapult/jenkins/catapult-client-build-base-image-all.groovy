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

		stage('build base images') {
			parallel {
				stage('gcc prior') {
					steps {
						script {
							dispatchBuildBaseImageJob(constants.gccPriorName, constants.ubuntuName, true)
						}
					}
				}
				stage('gcc latest') {
					steps {
						script {
							dispatchBuildBaseImageJob(constants.gccLatestName, constants.ubuntuName, true)
						}
					}
				}
				stage('gcc 10 [debian]') {
					steps {
						script {
							dispatchBuildBaseImageJob(constants.gccDebianName, constants.debianName, false)
						}
					}
				}
				stage('gcc westmere') {
					steps {
						script {
							dispatchBuildBaseImageJob(constants.gccWestmereName, constants.ubuntuName, true)
						}
					}
				}
				stage('gcc [fedora]') {
					steps {
						script {
							dispatchBuildBaseImageJob(constants.gccLatestName, constants.fedoraName, false)
						}
					}
				}

				stage('clang prior') {
					steps {
						script {
							dispatchBuildBaseImageJob(constants.clangPriorName, constants.ubuntuName, true)
						}
					}
				}
				stage('clang latest') {
					steps {
						script {
							dispatchBuildBaseImageJob(constants.clangLatestName, constants.ubuntuName, true)
						}
					}
				}

				stage('clang ausan') {
					steps {
						script {
							dispatchBuildBaseImageJob(constants.clangAusanName, constants.ubuntuName, false)
						}
					}
				}
				stage('clang tsan') {
					steps {
						script {
							dispatchBuildBaseImageJob(constants.clangTsanName, constants.ubuntuName, false)
						}
					}
				}

				stage('msvc latest') {
					steps {
						script {
							dispatchBuildBaseImageJob(constants.msvcLatestName, constants.windowsName, true)
						}
					}
				}
				stage('msvc prior') {
					steps {
						script {
							dispatchBuildBaseImageJob(constants.msvcPriorName, constants.windowsName, true)
						}
					}
				}

				stage('release base image') {
					steps {
						script {
							dispatchPrepareBaseImageJob('release', constants.ubuntuName)
						}
					}
				}

				stage('test base image') {
					steps {
						script {
							dispatchPrepareBaseImageJob(constants.testName, constants.ubuntuName)
						}
					}
				}
				stage('test base image [debian]') {
					steps {
						script {
							dispatchPrepareBaseImageJob(constants.testName, constants.debianName)
						}
					}
				}
				stage('test base image [fedora]') {
					steps {
						script {
							dispatchPrepareBaseImageJob(constants.testName, constants.fedoraName)
						}
					}
				}
				stage('test base image [windows]') {
					steps {
						script {
							dispatchPrepareBaseImageJob(constants.testName, constants.windowsName)
						}
					}
				}
			}
		}
	}
}

void dispatchBuildBaseImageJob(String compilerConfiguration, String operatingSystem, Boolean shouldBuildConanLayer) {
	build job: 'Symbol/server-pipelines/catapult-client-build-base-image', parameters: [
		string(name: 'COMPILER_CONFIGURATION', value: "${compilerConfiguration}"),
		string(name: constants.operatingSystemName, value: "${operatingSystem}"),
		string(name: 'SHOULD_BUILD_CONAN_LAYER', value: "${shouldBuildConanLayer}"),
		string(name: constants.manualGitBranchName, value: "${params.MANUAL_GIT_BRANCH}")
	]
}

void dispatchPrepareBaseImageJob(String imageType, String operatingSystem) {
	build job: 'Symbol/server-pipelines/catapult-client-prepare-base-image', parameters: [
		string(name: 'IMAGE_TYPE', value: "${imageType}"),
		string(name: constants.operatingSystemName, value: "${operatingSystem}"),
		string(name: constants.manualGitBranchName, value: "${params.MANUAL_GIT_BRANCH}")
	]
}
