import java.nio.file.Paths

void call(build_configuration, String git_url, String rootFolder, String credentialsId) {
    println("Build configuration: " + build_configuration)
    generate_multibranch_jobs(build_configuration, git_url, rootFolder, credentialsId)
}

// Create multibranch jobs for the monorepo
def generate_multibranch_jobs(build_configuration, String git_url, String root_folder, credentials_id) {
    create_jenkins_folder(root_folder, 'Generated Jobs')

    def tokens = git_url.tokenize('/')
    String repository_owner = tokens[2]
    String repository_name = tokens.last().split("\\.")[0]

    create_jenkins_folder("${root_folder}/${repository_name}", "Branch ${repository_name}" )

    String full_branch_folder = Paths.get(root_folder).resolve(repository_name).toString()
    generate_package_multibranch_jobs(build_configuration, full_branch_folder, git_url, repository_owner, repository_name, credentials_id)
    generate_repo_multibranch_job(build_configuration, full_branch_folder, git_url, repository_owner, repository_name, credentials_id)
}

// Create a multibranch job for each package in the monorepo
def generate_package_multibranch_jobs(build_configuration, String full_branch_folder, String repository_url, String repository_owner,
                                 String repository_name, String credentials_id) {

    build_configuration.builds.each { build ->
        String pipeline_name = build.path.tokenize('/').last()
        String job_name = Paths.get(full_branch_folder).resolve(pipeline_name).toString()
        String jenkinsfile_path = Paths.get(build.path).resolve("Jenkinsfile").toString()
        String package_include_paths = append_filter(build.path)

        if (build.depends_on != null) {
            build.depends_on.each { depends_on_path ->
                String filter_path = append_filter(depends_on_path)
                package_include_paths += "\n${filter_path}"
            }
        }
        String package_exclude_paths = ''
        create_multibranch_job(job_name, build.path, package_include_paths, package_exclude_paths, jenkinsfile_path, repository_url,
                repository_owner, repository_name, credentials_id)
    }
}

// Create a multibranch job which triggers for changes which are not covered by the package jobs.
def generate_repo_multibranch_job(build_configuration, String full_branch_folder, String repository_url, String repository_owner,
                                      String repository_name, String credentials_id) {
    String package_exclude_paths = ''
    build_configuration.builds.each { build ->
        package_exclude_paths += append_filter(build.path)
        if (build.depends_on != null) {
            build.depends_on.each { depends_on_path ->
                String filter_path = append_filter(depends_on_path)
                package_exclude_paths += "\n${filter_path}"
            }
        }
    }
    String pipeline_name = 'RootJob'
    String job_name = Paths.get(full_branch_folder).resolve(pipeline_name).toString()
    String jenkinsfile_path = "Jenkinsfile"
    String package_include_paths = ''
    create_multibranch_job(job_name, repository_name, package_include_paths, package_exclude_paths, jenkinsfile_path, repository_url,
            repository_owner, repository_name, credentials_id)
}


String append_filter(String path) {
    return path + "/**/*"
}

