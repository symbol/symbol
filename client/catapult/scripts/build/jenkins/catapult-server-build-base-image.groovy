pipeline {
    agent {
        label 'ubuntu-20.04-8cores-16Gig'
    }

    parameters {
        gitParameter branchFilter: 'origin/(.*)', defaultValue: 'main', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
        choice name: 'COMPILER_CONFIGURATION',
            choices: ['clang-latest', 'gcc-latest', 'clang-address-undefined', 'clang-thread', 'clang-11'],
            description: 'compiler configuration'
    }

    environment {
        DOCKER_URL = 'https://registry.hub.docker.com'
        DOCKER_CREDENTIALS_ID = 'docker-hub-token-symbolserverbot'
    }

    options {
        ansiColor('css')
        timestamps()
    }

    stages {
        stage('prepare') {
            stages {
                stage('prepare variables') {
                    steps {
                        script {
                            dest_image_name = "symbolplatform/symbol-server-build-base:${COMPILER_CONFIGURATION}"

                            base_image_dockerfile_generator_command = """
                                python3 ./scripts/build/baseImageDockerfileGenerator.py \
                                    --compiler-configuration scripts/build/configurations/${COMPILER_CONFIGURATION}.yaml \
                                    --versions ./scripts/build/versions.properties \
                            """
                        }
                    }
                }
                stage('print env') {
                    steps {
                        echo """
                                    env.GIT_BRANCH: ${env.GIT_BRANCH}
                                 MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}

                            COMPILER_CONFIGURATION: ${COMPILER_CONFIGURATION}

                                   dest_image_name: ${dest_image_name}
                        """
                    }
                }
            }
        }
        stage('build image') {
            stages {
                stage('build os') {
                    steps {
                        script {
                            build_and_push_layer('os', "${base_image_dockerfile_generator_command}")
                        }
                    }
                }
                stage('build boost') {
                    steps {
                        script {
                            build_and_push_layer('boost', "${base_image_dockerfile_generator_command}")
                        }
                    }
                }
                stage('build deps') {
                    steps {
                        script {
                            build_and_push_layer('deps', "${base_image_dockerfile_generator_command}")
                        }
                    }
                }
                stage('build test') {
                    steps {
                        script {
                            build_and_push_layer('test', "${base_image_dockerfile_generator_command}")
                        }
                    }
                }
                stage('build conan') {
                    when {
                        expression { "${COMPILER_CONFIGURATION}" == 'clang-latest' || "${COMPILER_CONFIGURATION}" == 'gcc-latest' }
                    }
                    steps {
                        script {
                            build_and_push_layer('conan', "${base_image_dockerfile_generator_command}")
                        }
                    }
                }
            }
        }
    }
}

def build_and_push_layer(layer, base_image_dockerfile_generator_command) {
    docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
        dest_image_name = sh(
            script: "${base_image_dockerfile_generator_command} --layer ${layer} --name-only",
            returnStdout: true
        ).trim()

        sh """
            ${base_image_dockerfile_generator_command} --layer ${layer} > Dockerfile

            echo "*** LAYER ${layer} => ${dest_image_name} ***"
            cat Dockerfile
        """

        docker.build("${dest_image_name}").push()
    }
}
