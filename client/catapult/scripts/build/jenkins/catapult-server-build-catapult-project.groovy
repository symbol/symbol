pipeline {
    agent {
        label 'cat-server-01'
    }

    parameters {
        choice name: 'COMPILER_CONFIGURATION',
            choices: ['clang-latest', 'gcc-latest', 'clang-address-undefined', 'clang-thread'],
            description: 'compiler configuration'
        choice name: 'BUILD_CONFIGURATION',
            choices: ['tests-conan', 'tests-metal', 'tests-diagnostics'],
            description: 'build configuration'
    }

    stages {
        stage('build') {
            steps {
                script {
                    sh 'echo "$(id -u):$(id -g)" > user.txt'
                    fully_qualified_user = readFile(file: 'user.txt').trim()

                    sh """
                        python3 scripts/build/runDockerBuild.py \
                            --compiler-configuration scripts/build/configurations/${COMPILER_CONFIGURATION}.yaml \
                            --build-configuration scripts/build/configurations/${BUILD_CONFIGURATION}.yaml \
                            --user ${fully_qualified_user}
                    """
                }
            }
        }
    }
}
