import java.nio.file.Paths

def call(String script_path, String filename) {
    String script_filepath = Paths.get(script_path, filename).toString()
    call(script_filepath)
}

def call(String script_filepath) {
    call(script_filepath, script_filepath, true)
}

def call(String script_filepath, String label='', Boolean return_stdout, Boolean return_status=false, String encoding='') {
    logger.log_info("Running script ${script_filepath}")
    if (isUnix()) {
        sh label: label, script: script_filepath, encoding: encoding, returnStdout: return_stdout, returnStatus: return_status
    }
    else {
        bat label: label, script: script_filepath, encoding: encoding, returnStdout: return_stdout, returnStatus: return_status
    }
}
