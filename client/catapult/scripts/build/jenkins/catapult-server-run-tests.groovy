pipeline {
    agent {
        label 'ubuntu-20.04-8cores-16Gig'
    }

    options {
        ansiColor('css')
    }

    parameters {
        string name: 'TEST_IMAGE_NAME', description: 'docker test image name', defaultValue: 'catapult-server-test-images-3..'
        choice name: 'TEST_MODE',
            choices: ['bench', 'test'],
            description: 'test mode'
        choice name: 'VERBOSITY',
            choices: ['test', 'suite', 'max'],
            description: 'output verbosity level'
    }

    stages {
        stage('build') {
            steps {
                script {
                    sh 'echo "$(id -u):$(id -g)" > user.txt'
                    fully_qualified_user = readFile(file: 'user.txt').trim()

                    docker.withRegistry('https://registry.hub.docker.com', 'docker-hub-token-symbolserverbot') {
                        docker.image("symbolplatform/symbol-server-test:${TEST_IMAGE_NAME}").pull()

                        sh """
                            python3 catapult-src/scripts/build/runDockerTests.py \
                                --image ${TEST_IMAGE_NAME} \
                                --user ${fully_qualified_user} \
                                --mode ${TEST_MODE} \
                                --verbosity ${VERBOSITY}
                        """
                    }
                }
            }
        }
    }
}
