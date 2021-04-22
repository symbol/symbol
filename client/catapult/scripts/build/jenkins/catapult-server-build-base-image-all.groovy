pipeline {
    agent any

    parameters {
        gitParameter branchFilter: 'origin/(.*)', defaultValue: 'main', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
    }

    stages {
        stage('build base images') {
            parallel {
                stage('gcc-10') {
                    steps {
                        script {
                            dispatch_base_image_job('gcc-latest')
                        }
                    }
                }
                stage('clang-11') {
                    steps {
                        script {
                            dispatch_base_image_job('clang-latest')
                        }
                    }
                }
                stage('clang-11 ausan') {
                    steps {
                        script {
                            dispatch_base_image_job('clang-address-undefined')
                        }
                    }
                }
                stage('clang-11 tsan') {
                    steps {
                        script {
                            dispatch_base_image_job('clang-thread')
                        }
                    }
                }
                stage('test base image') {
                    steps {
                        build job: 'server-pipelines/catapult-server-prepare-test-base-image', parameters: [
                            string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}")
                        ]
                    }
                }
            }
        }
    }
}

def dispatch_base_image_job(compiler_configuration) {
    build job: 'server-pipelines/catapult-server-build-base-image', parameters: [
        string(name: 'COMPILER_CONFIGURATION', value: "${compiler_configuration}"),
        string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}")
    ]
}
