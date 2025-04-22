# File Matcher & Counter

This program recursively searches a specified base directory (and its subdirectories) for files that match a given pattern (e.g., files ending in `.c`) and whose sizes fall within a specified range. For every matching file, the program writes a detailed listing (using `ls -lh`) to an output file (`res.txt`) and, at the end, appends a summary count of all matching files.

## Features

- **Recursive Search:** Scans the base directory and all its subdirectories.
- **Pattern Matching:** Filters files based on a user-specified file extension or pattern.
- **Size Filtering:** Only considers files whose sizes are between a minimum and maximum value. Sizes can be specified in bytes, kilobytes (kB), megabytes (MB), or gigabytes (GB).
- **Multithreading:** Uses 4 threads (by default) to distribute directory processing across CPU cores.
- **Graceful Termination:** Handles `SIGINT` (Ctrl+C) so that a final count is written even if the program is interrupted.

## Requirements

- A C compiler (e.g., `gcc`)
- POSIX threads support (typically available on Unix-like systems)
- A Unix-like operating system (Linux, macOS, etc.)

## Installation

You can obtain the program in one of three ways:

### Clone via HTTPS

```bash
git clone https://github.com/Erik-Galstyan/your-repo-name.git
```

### Clone via SSH

```bash
git clone git@github.com:Erik-Galstyan/your-repo-name.git
```
### Download as ZIP

1. Go to the repository page on GitHub.
2. Click the **Code** button.
3. Select **Download ZIP.**
4. Extract the ZIP file to your desired location.

## Compilation
Navigate to the project directory and compile the program using gcc with the pthread library. For example:
```bash
gcc -pthread -o file_matcher file_matcher.c
```
## Usage
Run the program with the following command-line arguments:

```bash
./file_matcher --base-dir <directory> --pattern <pattern> --min-size <min> --max-size <max>
```
## Authors
- [Erik Galstyan](https://github.com/Erik-Galstyan)

