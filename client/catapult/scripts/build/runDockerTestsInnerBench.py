import argparse
from pathlib import Path

from environment import EnvironmentManager
from process import ProcessManager


def main():
    parser = argparse.ArgumentParser(description='catapult bench runner')
    parser.add_argument('--exe-path', help='path to executables', required=True)
    parser.add_argument('--out-dir', help='directory in which to store bench output files', required=True)
    parser.add_argument('--dry-run', help='outputs desired commands without runing them', action='store_true')
    args = parser.parse_args()

    process_manager = ProcessManager(args.dry_run)
    environment_manager = EnvironmentManager(args.dry_run)

    for filepath in environment_manager.find_glob(args.exe_path, 'bench*'):
        output_filepath = Path(args.out_dir) / (filepath.name + '.json')
        process_manager.dispatch_subprocess([filepath, '--benchmark_out=' + str(output_filepath)])


if __name__ == '__main__':
    main()
