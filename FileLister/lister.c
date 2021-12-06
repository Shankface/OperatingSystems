#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h> 
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>


void printInfo(char *path, struct stat st){

    char *type = "";
    bool sym_exists = false;
    char buf[PATH_MAX];
    char sym[PATH_MAX + 4] = " -> ";

    //deciphering inode type
    switch (st.st_mode&S_IFMT){
        case S_IFDIR:  type = "d"; break;
        case S_IFBLK:  type = "-"; break;
        case S_IFCHR:  type = "-"; break;
        case S_IFIFO:  type = "-"; break;
        case S_IFLNK:  //if its a symlink
            type = "l"; 
            if((readlink(path, buf, PATH_MAX)) == -1) perror("ERROR with readlink"); 
            strcat(sym, buf); 
            sym_exists = true; 
            break;
        case S_IFREG:  type = "-"; break;
        case S_IFSOCK: type = "-"; break;
        default:       type = "?"; printf("ERROR: Unknown inode type.\n"); break;
    }

    //deciphering permissions
    char mode[10];
    int i;
    int current = 0400; 
    char vals[] = {'r','w','x'};
    for(i = 0; i < 9; i++){
        mode[i] = ((st.st_mode&0777)&current) == 0 ? '-' : vals[i%3];
        current = current >> 1;
    }
    //sticky bits
    if(st.st_mode & S_ISUID) mode[2] = (st.st_mode & S_IXUSR) ? 's' : 'S';
    if(st.st_mode & S_ISGID) mode[5] = (st.st_mode & S_IXGRP) ? 's' : 'l';
    if(st.st_mode & S_ISVTX) mode[8] = (st.st_mode & S_IXOTH) ? 't' : 'T';
    mode[9] = '\0';

    //getting user ID
    char *uid;
    struct passwd *pwd = getpwuid(st.st_uid);
    if(pwd != NULL) uid = pwd->pw_name;
    else{
        uid = "unknown";
        perror("ERROR with getpwuid");
    }

    //getting group ID
    char *gid;
    struct group *grp = getgrgid(st.st_gid);
    if(grp != NULL) gid = grp->gr_name;
    else{
        gid = "unknown"; 
        perror("ERROR with getgrgid");
    }

    char lastMod[40];
    strftime(lastMod, 40, "%b %e %H:%M", localtime(&st.st_mtime)); //formatting date and time

    printf("%2d %4d %s%s  %2d %s %s  %10d %10s %s%s\n", st.st_ino, st.st_blocks/2, type, mode, st.st_nlink, uid, gid, st.st_size, lastMod, path, sym_exists ? sym : ""); //printting info to standart out

}

//recursive filesystem lister
void dir_list(char *dr){
    
    DIR *dir = opendir(dr);
    struct dirent *dirEntry;
    struct stat st;
    char path[4096];

    if(lstat(dr, &st) == -1) perror("ERROR with lstat"); //print error for lstat
    printInfo(dr, st);

    if((dir = opendir(dr)) != NULL){
        errno = 0;
        while(dirEntry = readdir(dir)){

            if((strcmp(dirEntry->d_name, ".")) != 0 && (strcmp(dirEntry->d_name, "..") != 0)){ //try to open directory as long as it isn't itself or parent
            
                strcpy(path, dr);
                strcat(path, "/");
                strcat(path, dirEntry->d_name);
                if(lstat(path, &st) == -1) perror("ERROR with lstat");           

                dirEntry->d_type == DT_DIR ? dir_list(path) : printInfo(path, st); //only try to open as directory if the type is directory

            }
        }
        if(errno) perror("ERROR reading directory: ");
    }

    else perror("ERROR opening directory");

    if(closedir(dir) != 0) perror("ERROR closing directory"); 
}

int main(int argc, char *argv[]){
    dir_list((argc > 1) ? argv[1] : "."); //list itself if no directory is specified
    return 0;
}