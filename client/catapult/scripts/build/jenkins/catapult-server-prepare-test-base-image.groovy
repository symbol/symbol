pipeline {
    agent {
        label 'cat-server-01'
    }

    stages {
        stage('prepare Dockerfile') {
            steps {
                script {
                    ubuntu_version = readProperties(file: './scripts/build/versions.properties')['ubuntu']

                    dockerfile_template = './scripts/build/templates/TestBaseImage.Dockerfile'
                    dockerfile_contents = readFile(file: dockerfile_template)
                    dockerfile_contents = dockerfile_contents.replaceAll('\\{\\{BASE_IMAGE\\}\\}', "ubuntu:${ubuntu_version}")

                    writeFile(file: 'Dockerfile', text: dockerfile_contents)
                }
            }
        }
        stage('build image') {
            steps {
                script {
                    sh """
                        echo "*** Dockerfile ***"
                        cat Dockerfile
                    """

                    docker_image = docker.build "symbolplatform/catapult-test-base:latest"
                    docker_image.push()
                }
            }
        }
    }
}
