pipeline {
    agent any

    stages {
        stage('build base images') {
            parallel {
                stage('gcc-10') {
                    steps {
                        script {
                            dispatch_base_image_job('gcc-10', '')
                        }
                    }
                }
                stage('clang-11') {
                    steps {
                        script {
                            dispatch_base_image_job('clang-11', '')
                        }
                    }
                }
                stage('clang-11 ausan') {
                    steps {
                        script {
                            dispatch_base_image_job('clang-11', 'address,undefined')
                        }
                    }
                }
                stage('clang-11 tsan') {
                    steps {
                        script {
                            dispatch_base_image_job('clang-11', 'thread')
                        }
                    }
                }
                stage('test base image') {
                    steps {
                        build job: "catapult-server-prepare-test-base-image/${env.BRANCH_NAME}", wait: false
                    }
                }
            }
        }
    }
}

def dispatch_base_image_job(compiler, sanitizers) {
    build job: "catapult-server-build-base-image/${env.BRANCH_NAME}", wait: false, parameters: [
        string(name: 'COMPILER', value: "${compiler}"),
        string(name: 'SANITIZERS', value: "${sanitizers}")
    ]
}
