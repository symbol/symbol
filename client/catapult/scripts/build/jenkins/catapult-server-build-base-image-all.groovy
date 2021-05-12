pipeline {
    agent any

    parameters {
        gitParameter branchFilter: 'origin/(.*)', defaultValue: 'main', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
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
                stage('gcc latest') {
                    steps {
                        script {
                            dispatch_build_base_image_job('gcc-latest')
                        }
                    }
                }
                stage('clang latest') {
                    steps {
                        script {
                            dispatch_build_base_image_job('clang-latest')
                        }
                    }
                }
                stage('clang ausan') {
                    steps {
                        script {
                            dispatch_build_base_image_job('clang-address-undefined')
                        }
                    }
                }
                stage('clang tsan') {
                    steps {
                        script {
                            dispatch_build_base_image_job('clang-thread')
                        }
                    }
                }
                stage('clang 11') {
                    steps {
                        script {
                            dispatch_build_base_image_job('clang-11')
                        }
                    }
                }

                stage('release base image') {
                    steps {
                        script {
                            dispatch_prepare_base_image_job('release')
                        }
                    }
                }
                stage('test base image') {
                    steps {
                        script {
                            dispatch_prepare_base_image_job('test')
                        }
                    }
                }
            }
        }
    }
}

def dispatch_build_base_image_job(compiler_configuration) {
    build job: 'server-pipelines/catapult-server-build-base-image', parameters: [
        string(name: 'COMPILER_CONFIGURATION', value: "${compiler_configuration}"),
        string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}")
    ]
}

def dispatch_prepare_base_image_job(image_type) {
    build job: 'server-pipelines/catapult-server-prepare-base-image', parameters: [
        string(name: 'IMAGE_TYPE', value: "${image_type}"),
        string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}")
    ]
}

