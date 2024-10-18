#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

void print_file_info(const char* filename, struct stat *fileStat) {
    char timebuf[80];
    struct passwd *pw = getpwuid(fileStat->st_uid);
    struct group *gr = getgrgid(fileStat->st_gid);
    struct tm *tm_info = localtime(&fileStat->st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);

    printf((S_ISDIR(fileStat->st_mode)) ? "d" : "-");
    printf((fileStat->st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat->st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat->st_mode & S_IXUSR) ? "x" : "-");
    printf((fileStat->st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat->st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat->st_mode & S_IXGRP) ? "x" : "-");
    printf((fileStat->st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat->st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat->st_mode & S_IXOTH) ? "x" : "-");
    printf(" %u %s %s %5ld %s %s\n", fileStat->st_nlink, pw->pw_name, gr->gr_name, fileStat->st_size, timebuf, filename);
}

int compare_mtime(const struct dirent **a, const struct dirent **b) {
    struct stat statA, statB;
    stat((*a)->d_name, &statA);
    stat((*b)->d_name, &statB);
    return statB.st_mtime - statA.st_mtime; // 최신 수정 시간 기준 내림차순 정렬
}

void list_directory(int aflag, int lflag, int tflag) {
    DIR *d;
    struct dirent *dir;
    struct dirent **namelist;
    struct stat fileStat;
    int n;

    d = opendir(".");

    if (d) {
        // 파일 목록을 배열로 정렬하여 처리
        n = scandir(".", &namelist, NULL, tflag ? compare_mtime : alphasort);
        if (n < 0) {
            perror("scandir");
        } else {
            while (n--) {
                dir = namelist[n];
                if (!aflag && dir->d_name[0] == '.') {
                    free(namelist[n]);
                    continue;
                }
                if (lflag) {
                    stat(dir->d_name, &fileStat);
                    print_file_info(dir->d_name, &fileStat);
                } else {
                    printf("%s\n", dir->d_name);
                }
                free(namelist[n]);
            }
            free(namelist);
        }
        closedir(d);
    } else {
        perror("opendir");
    }
}

int main(int argc, char *argv[]) {
    int aflag = 0, lflag = 0, tflag = 0;
    char input[100];
    char *token;

    while (1) {
        aflag = 0; lflag = 0; tflag = 0;
        printf("$ ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) {
                printf("터미널을 종료합니다...");
                break;
        }

        token = strtok(input, " ");
        if (token != NULL && strcmp(token, "ls") == 0) {
                token = strtok(NULL, " ");
                while (token != NULL) {
                        if (strcmp(token, "-a") == 0) {
                                aflag = 1;
                        } else if (strcmp(token, "-l") == 0) {
                                lflag = 1;
                        } else if (strcmp(token, "-t") == 0) {
                                tflag = 1;
                        } else if (strcmp(token, "-al") == 0 || strcmp(token, "-la") == 0) {
                                aflag = 1;
                                lflag = 1;
                        }
                        token = strtok(NULL, " ");
                }
                list_directory(aflag, lflag, tflag);
        } else {
                printf("Unknown command: %s\n", input);
        }
    }

    return 0;
}
