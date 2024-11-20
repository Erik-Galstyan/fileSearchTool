#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <bits/pthreadtypes.h>
#include <features.h>
#include <sched.h>
#include <time.h>
#include <bits/types.h>
#include <signal.h>

#define BASEDIRSIZE 100
#define PATTERNSIZE 50
#define MINMAXSIZE 20
#define BASEDIRCOMSIZE 20
#define PATTERNCOMSIZE 50
#define MINMAXCOMSIZE 20
#define COMPUTERCORESIZE 4

int count_of_missing_file;
int Size = 1;
int Index = 1;
char **Dir_array;
char base_dir[BASEDIRSIZE];
char pattern[PATTERNSIZE];
char min_size[MINMAXSIZE];
char max_size[MINMAXSIZE];
char base_dir_command[BASEDIRCOMSIZE];
char pattern_command[PATTERNCOMSIZE];
char min_size_command[MINMAXCOMSIZE];
char max_size_command[MINMAXCOMSIZE];
unsigned long long min;
unsigned long long max;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void end(void){
    char last_command[1024] = {0};
    snprintf(last_command,sizeof(last_command),"echo Count of matching files in %d directories: %d >> res.txt",Size,count_of_missing_file);
    system(last_command);
}


void handler(int signum) {
    atexit(end);
    exit(EXIT_FAILURE);
}

unsigned long long convert_to_bytes(char* memory) {

  unsigned long long memSize = atoi(memory);

  if (strcasestr(memory,"gb")) {
    return memSize * 1024 * 1024 * 1024;
  } else if (strcasestr(memory,"kB")) {
    return memSize * 1024;
  } else if (strcasestr(memory,"mb")) {
    return memSize * 1024 * 1024;
  } else if ( strcasestr(memory,"b")) {
    return memSize;
  }
  
  
  
}

void add_directory(char **dir_array, const char *dir_path) {
    strcpy(dir_array[Index], dir_path);
    Index++;
}

void finding_directories_and_putting_in_array(const char *dir_path, char **dir_array) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;

    dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char path[1024];

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        if (stat(path, &statbuf) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            add_directory(dir_array, path);
            finding_directories_and_putting_in_array(path, dir_array);
        }
    }

    if (closedir(dir) == -1) {
        perror("closedir");
    }
}

void find_count_directories(const char *dir_path) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;

    dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char path[2048];

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        if (stat(path, &statbuf) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            Size++;
            find_count_directories(path);
        }
    }

    if (closedir(dir) == -1) {
        perror("closedir");
    }
}

typedef struct Threads {
    int start;
    int howMuch;
} Threads;

void *thread_func(void *arg) {
    Threads threadObj = *(Threads *)arg;
    int start = threadObj.start;
    int end = start + threadObj.howMuch;
    printf("min = %lld max = %lld\n", min, max);
    for (int i = start; i < end; i++) {
        DIR *dir;
        struct dirent *entry;
        struct stat statbuf;

        dir = opendir(Dir_array[i]);
        if (dir == NULL) {
            perror("opendir");
            exit(EXIT_FAILURE);
        }

        char command[2048] = {0};

        while ((entry = readdir(dir)) != NULL) {
            char full_path[2048];
            snprintf(full_path, sizeof(full_path), "%s/%s", Dir_array[i], entry->d_name);

            if (stat(full_path, &statbuf) == -1) {
                perror("stat");
                continue;
            }

            if (S_ISDIR(statbuf.st_mode)) {
                continue;
            }

            int size = strlen(entry->d_name);
            int patternSize = strlen(pattern);
            int Wrongflag = 0;

            if (patternSize > size) {
                continue;
            }

            for (int j = 0, k = size - patternSize; j < patternSize; j++, k++) {
                if (entry->d_name[k] != pattern[j]) {
                    Wrongflag = 1;
                    break;
                }
            }

            if (Wrongflag || statbuf.st_size < min || statbuf.st_size > max) {
                continue;
            }

            snprintf(command, sizeof(command), "ls -lh %s >> res.txt", full_path);
            system(command);

            pthread_mutex_lock(&mutex);
            count_of_missing_file++;
            pthread_mutex_unlock(&mutex);
        }

        if (closedir(dir) == -1) {
            perror("closedir");
        }
    }

    return NULL;
}
int main(int argc, char *argv[]) {

    signal(SIGINT,handler);    

    if (argc != 9) {
        fprintf(stderr, "Usage : %s --base-dir <dir> --pattern <pattern> --min-size <min> --max-size <max>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    strcpy(base_dir, argv[2]);
    strcpy(pattern, argv[4]);
    strcpy(min_size, argv[6]);
    strcpy(max_size, argv[8]);
    strcpy(base_dir_command, argv[1]);
    strcpy(pattern_command, argv[3]);
    strcpy(min_size_command, argv[5]);
    strcpy(max_size_command, argv[7]);
    printf("min size = %s\n",min_size);
    printf("max size = %s\n",max_size);
    

    min = convert_to_bytes(min_size);
    max = convert_to_bytes(max_size);
    // printf("min = %lld\nmax = %lld",min,max);

    if (strcmp(base_dir_command, "--base-dir") || strcmp(pattern_command, "--pattern") || strcmp(min_size_command, "--min-size") || strcmp(max_size_command, "--max-size")) {
        printf("Your commands are wrong!\nCorrect usage : %s --base-dir <dir> --pattern <pattern> --min-size <min> --max-size <max>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    find_count_directories(base_dir);

    // printf("directory is :%d\n",Size);

    Dir_array = (char **)calloc(Size, sizeof(char *));
    for (int i = 0; i < Size; i++) {
        Dir_array[i] = (char *)calloc(1024, 1);
    }

    finding_directories_and_putting_in_array(base_dir, Dir_array);
    strcpy(Dir_array[0],base_dir);

    int threadWorkSize = Size / COMPUTERCORESIZE;
    int threadStart = 0;

    Threads thread[COMPUTERCORESIZE];
    pthread_t tid[COMPUTERCORESIZE];

    for (int i = 0; i < COMPUTERCORESIZE; i++) {
        thread[i].start = threadStart;
        thread[i].howMuch = (i == COMPUTERCORESIZE - 1) ? (Size - threadStart) : threadWorkSize;
        threadStart += threadWorkSize;

        if (pthread_create(&tid[i], NULL, thread_func, &thread[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // for (int i = 0; i < Size; i++) {
    //   printf("dir names:%s\n",Dir_array[i]);
    // }

    for (int i = 0; i < COMPUTERCORESIZE; i++) {
        if (pthread_join(tid[i], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }
    char last_command[1024] = {0};
    snprintf(last_command,sizeof(last_command),"echo Count of matching files in %d directories: %d >> res.txt",Size,count_of_missing_file);
    system(last_command);

    for (int i = 0; i < Size; i++) {
        free(Dir_array[i]);
    }
    free(Dir_array);

    return 0;
}
