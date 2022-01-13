import java.nio.file.Paths

String create_temp_path(String filepath) {
    return Paths.get("${WORKSPACE_TMP}", filepath).toString()
}

String copy_to_temp_file(String data, String destination_filepath) {
    return copy_to_local_file(data, create_temp_path(destination_filepath))
}

String copy_to_local_file(String data, String destination_filepath) {
    String parent_path = Paths.get(destination_filepath).getParent().toString()
    dir (parent_path) {
        writeFile(file: destination_filepath, text: data)
        logger.log_info("Wrote data to ${destination_filepath}")
    }
    return destination_filepath
}