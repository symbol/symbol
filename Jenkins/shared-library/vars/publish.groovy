void call(config, String phase) {
    logger.log_info("publishing data for ${phase}, ${config}")

    publisher(config, phase)
}

String get_resource(String file_name) {
    return libraryResource(file_name)
}

def docker_publisher(config, phase) {
    if (config.docker_image_name) {
        env.DOCKER_IMAGE_NAME = config.docker_image_name
        fileHelper.copy_to_temp_file(get_resource('publish_artifacts/scripts/Jenkins-functions.sh'), 'scripts/Jenkins-functions.sh')
        fileHelper.copy_to_temp_file(get_resource('publish_artifacts/scripts/docker-functions.sh'), 'scripts/docker-functions.sh')
        String script_full_filepath = fileHelper.copy_to_temp_file(get_resource('publish_artifacts/scripts/docker-publish.sh'), 'scripts/docker-publish.sh')
        withCredentials([usernamePassword(credentialsId: DOCKERHUB_CREDENTIALS_ID, usernameVariable: 'DOCKER_USERNAME', passwordVariable: 'DOCKER_PASSWORD')]) {
            run_script("bash ${script_full_filepath} ${phase}")
        }
    }
}

def npm_publisher(config, phase) {
    if (config.npm) {
        fileHelper.copy_to_temp_file(get_resource('publish_artifacts/scripts/Jenkins-functions.sh'), 'scripts/Jenkins-functions.sh')
        fileHelper.copy_to_temp_file(get_resource('publish_artifacts/scripts/node-functions.sh'), 'scripts/node-functions.sh')
        String script_full_filepath = fileHelper.copy_to_temp_file(get_resource('publish_artifacts/scripts/node-publish.sh'), 'scripts/node-publish.sh')
        fileHelper.copy_to_local_file(get_resource('publish_artifacts/configuration/npmrc'), "${HOME}/.npmrc")
        withCredentials([string(credentialsId: NPM_CREDENTIALS_ID, variable: 'NPM_TOKEN')]) {
            run_script("bash ${script_full_filepath} ${phase}")
        }
    }
}

void publisher(config, phase) {
    Closure[] strategies = [
        this.&docker_publisher,
        this.&npm_publisher
    ]

    strategies.each { publisher ->
        publisher.call(config, phase)
    }
}