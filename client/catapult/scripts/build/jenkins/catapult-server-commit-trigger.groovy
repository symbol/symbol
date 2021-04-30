pipeline {
    agent any

    parameters {
        gitParameter branchFilter: 'origin/(.*)', defaultValue: 'main', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
    }

    stages {
        stage('build') {
            steps {
                script {
                    sh 'printenv'
                    dispatch_build_job('gcc-latest', 'tests-metal')
                }
            }
        }
    }
}

def dispatch_build_job(compiler_configuration, build_configuration) {
    git_branch = env.GIT_BRANCH ?: MANUAL_GIT_BRANCH
    build job: 'server-pipelines/catapult-server-build-catapult-project', parameters: [
        string(name: 'COMPILER_CONFIGURATION', value: "${compiler_configuration}"),
        string(name: 'BUILD_CONFIGURATION', value: "${build_configuration}"),
        gitParameter(name: 'MANUAL_GIT_BRANCH', value: "${git_branch}")
    ]
}
