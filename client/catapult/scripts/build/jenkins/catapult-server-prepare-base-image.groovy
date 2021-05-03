pipeline {
    agent {
        label 'cat-server-01'
    }

    parameters {
        gitParameter branchFilter: 'origin/(.*)', defaultValue: 'main', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
        choice name: 'IMAGE_TYPE',
            choices: ['release', 'test'],
            description: 'image type'
    }

    stages {
        stage('prepare Dockerfile') {
            steps {
                script {
                    ubuntu_version = readProperties(file: './scripts/build/versions.properties')['ubuntu']

                    dockerfile_template = "./scripts/build/templates/${params.IMAGE_TYPE.capitalize()}BaseImage.Dockerfile"
                    dockerfile_contents = readFile(file: dockerfile_template)
                    dockerfile_contents = dockerfile_contents.replaceAll('\\{\\{BASE_IMAGE\\}\\}', "ubuntu:${ubuntu_version}")

                    writeFile(file: 'Dockerfile', text: dockerfile_contents)
                }
            }
        }

        stage('build image') {
            steps {
                script {
                    sh '''
                        echo '*** Dockerfile ***'
                        cat Dockerfile
                    '''

                    docker_image = docker.build "symbolplatform/symbol-server-${params.IMAGE_TYPE}-base:latest"
                    docker.withRegistry('https://registry.hub.docker.com', 'docker-hub-token-symbolserverbot') {
                        docker_image.push()
                    }
                }
            }
        }
    }
}
