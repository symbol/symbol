void call(String script_filepath) {
    logger.log_info("Calling linter ${script_filepath}")
    run_script("bash ${script_filepath}")
}
