pipeline {
	agent any

	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
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
							dispatch_build_base_image_job('gcc-prior', 'ubuntu', true)
						}
					}
				}
				stage('gcc latest') {
					steps {
						script {
							dispatch_build_base_image_job('gcc-latest', 'ubuntu', true)
						}
					}
				}
				stage('gcc 10 [debian]') {
					steps {
						script {
							dispatch_build_base_image_job('gcc-10', 'debian', false)
						}
					}
				}
				stage('gcc westmere') {
					steps {
						script {
							dispatch_build_base_image_job('gcc-westmere', 'ubuntu', true)
						}
					}
				}
				stage('gcc [fedora]') {
					steps {
						script {
							dispatch_build_base_image_job('gcc-latest', 'fedora', false)
						}
					}
				}

				stage('clang prior') {
					steps {
						script {
							dispatch_build_base_image_job('clang-prior', 'ubuntu', true)
						}
					}
				}
				stage('clang latest') {
					steps {
						script {
							dispatch_build_base_image_job('clang-latest', 'ubuntu', true)
						}
					}
				}

				stage('clang ausan') {
					steps {
						script {
							dispatch_build_base_image_job('clang-ausan', 'ubuntu', false)
						}
					}
				}
				stage('clang tsan') {
					steps {
						script {
							dispatch_build_base_image_job('clang-tsan', 'ubuntu', false)
						}
					}
				}

				stage('release base image') {
					steps {
						script {
							dispatch_prepare_base_image_job('release', 'ubuntu')
						}
					}
				}

				stage('test base image') {
					steps {
						script {
							dispatch_prepare_base_image_job('test', 'ubuntu')
						}
					}
				}
				stage('test base image [debian]') {
					steps {
						script {
							dispatch_prepare_base_image_job('test', 'debian')
						}
					}
				}
				stage('test base image [fedora]') {
					steps {
						script {
							dispatch_prepare_base_image_job('test', 'fedora')
						}
					}
				}
			}
		}
	}
}

def dispatch_build_base_image_job(compiler_configuration, operating_system, should_build_conan_layer) {
	build job: 'Symbol/server-pipelines/catapult-client-build-base-image', parameters: [
		string(name: 'COMPILER_CONFIGURATION', value: "${compiler_configuration}"),
		string(name: 'OPERATING_SYSTEM', value: "${operating_system}"),
		string(name: 'SHOULD_BUILD_CONAN_LAYER', value: "${should_build_conan_layer}"),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}")
	]
}

def dispatch_prepare_base_image_job(image_type, operating_system) {
	build job: 'Symbol/server-pipelines/catapult-client-prepare-base-image', parameters: [
		string(name: 'IMAGE_TYPE', value: "${image_type}"),
		string(name: 'OPERATING_SYSTEM', value: "${operating_system}"),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}")
	]
}

