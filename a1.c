#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <fcntl.h>


#define PATH 4096
#define BUFFER 10000

#define MIN_VERS 37
#define MAX_VERS 61

#define MIN_SECT 7
#define MAX_SECT 14


struct {
    char name[15];
    int type;
    int offset;
    int size;
}sections[400];



int has_write_permission(const char *entry_path) 
{
    struct stat entry_stat;
    if (lstat(entry_path, &entry_stat) == -1) {
        printf("Error\n");
        return 0;
    }

    return (entry_stat.st_mode & S_IWUSR);
}

int is_valid_entry(const struct dirent *entry) 
{
    return strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0;
}

char *get_entry_path(const char *dir_name, const struct dirent *entry) 
{
    char *e_path = malloc(PATH);
    if (e_path == NULL) 
    {
        printf("Error\n");
        return NULL;
    }

    snprintf(e_path, PATH, "%s/%s", dir_name, entry->d_name);

    return e_path;
    free(e_path);
}

int validate_magic_number(int fd)
{
    char magic_buffer[1];
    lseek(fd, -1, SEEK_END);
    read(fd, magic_buffer, 1);
    if(magic_buffer[0] != 'n')
    {
        printf("ERROR\nwrong magic\n");
        return -1;
    }
    return 0;
}

int validate_header_size(int fd, int* header_size) 
{

    if (lseek(fd, -3, SEEK_END) == -1) 
    {
        printf("ERROR\nlseek failed\n");
        return -1;
    }
    if (read(fd, header_size, 2) != 2) 
    {
        printf("ERROR\nread failed\n");
        return -1;
    }
    
    return 0;
}

int validate_version(int fd, int header_size, int* version) 
{

    if (lseek(fd, -header_size, SEEK_END) == -1) 
    {
        printf("ERROR\nlseek failed\n");
        return -1;
    }
    
    if (read(fd, version, 4) != 4) 
    {
        printf("ERROR\nread failed\n");
        return -1;
    }
    
    return 0;
}
void parse(char* path)
{
    int fd=  open(path,O_RDONLY);
    if(fd==-1)
    {
        printf("Error\n");
        return;
    }

    int validate = validate_magic_number(fd);
    if(validate !=0)
    {
        return;
    }
    int header_size;

    if (validate_header_size(fd, &header_size) != 0) 
    {
    return;
    }

    int version;

    if (validate_version(fd, header_size, &version) != 0) 
    {
    return;
    }


    if(version < MIN_VERS || version > MAX_VERS)
    {
        printf("ERROR\nwrong version\n");
    
        return;
    }

    int nr_of_sections=0;
    read(fd,&nr_of_sections,1);

    if(nr_of_sections<MIN_SECT|| nr_of_sections>MAX_SECT)
    {
        printf("ERROR\nwrong sect_nr\n");
        return;
    }

    for (int i=0; i < nr_of_sections;i++)
    {
        read(fd,sections[i].name,14);

        sections[i].name[15] ='\0';

        read(fd,&sections[i].type,4);

        if(sections[i].type != 95 && sections[i].type != 45 && sections[i].type != 62)
        {
            printf("ERROR\nwrong sect_types\n");
            return;
        }
        read(fd,&sections[i].offset,sizeof(int));

        read(fd,&sections[i].size,sizeof(int));
    }

    printf("SUCCESS\n");

    printf("version=%d\n",version);

    printf("nr_sections=%d\n",nr_of_sections);

    for(int i =0; i<nr_of_sections;i++)
    {
        printf("section%d: %s %d %d\n",i+1,sections[i].name,sections[i].type,sections[i].size);
    }
    close(fd);
}


void list(const char *dir_name, int size, int has_perm_write) 
{ 
    DIR *dir = opendir(dir_name);
    if (dir == NULL) 
    {
    
        printf("Error\n");
        return;
    }

    printf("SUCCESS\n");

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) 
    {
        if (!is_valid_entry(entry)) 
        {
            continue;
        }

        char *e_path = get_entry_path(dir_name, entry);
        if (e_path == NULL) 
        {
            continue;
        }


        struct stat entry_stat;
        if (lstat(e_path, &entry_stat) == -1) 
        {
            printf("Error\n");
            free(e_path);

            continue;
        }

        if (entry_stat.st_size > size && (has_perm_write == 0 || has_write_permission(e_path))) 
        {
            printf("%s\n", e_path);
        }

        free(e_path);
    }

    closedir(dir);
}

void list_recursive(const char *dir_name, int size, int has_perm_write, const char *exclude_dir) 
{
    DIR *dir = opendir(dir_name);
    if (dir == NULL) 
    {
        printf("Error\n");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (!is_valid_entry(entry)) 
        {
            continue;
        }

        if (strcmp(entry->d_name, exclude_dir) == 0)
         {

            continue; // exclude the specified directory from output
        }

        char *e_path = get_entry_path(dir_name, entry);

        if (e_path == NULL) 
        {
            continue;
        }

        struct stat entry_stat;

        if (lstat(e_path, &entry_stat) == -1) 
        {
            printf("Error\n");
            free(e_path);
            continue;
        }
        char source_path[5000];

        snprintf(source_path, PATH, "%s/%s", e_path, entry->d_name);

        if (S_ISLNK(entry_stat.st_mode)) 
        {
            char link_target[PATH];
            ssize_t len = readlink(e_path, link_target, sizeof(link_target)-1);
            if (len == -1) 
            {

                printf("Error\n");
                free(e_path);
                continue;
            }
            link_target[len] = '\0';
            printf("%s -> %s\n", e_path, link_target);

        } else if (S_ISREG(entry_stat.st_mode)) 
        {
            if (entry_stat.st_size > size && (has_perm_write == 0 || has_write_permission(e_path))) 
            {
                printf("%s\n", e_path);
            }

        } else if (S_ISDIR(entry_stat.st_mode)) 
        {
            if(size == 0)
            {
            printf("%s\n", e_path);
            }

            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
            {
                list_recursive(e_path, size, has_perm_write, exclude_dir);
            }
        }

        free(e_path);
    }


    closedir(dir);
}

int main(int argc, char **argv) 
{
    int recursive = 0;
    int has_perm_write = 0;
    int filter_size_greater = 0;
    char *dir_path = NULL;
    int found_list = 0;
    int i = 1;

    while (i < argc) 
    {
        if (strcmp(argv[i], "variant") == 0) 
        {
            printf("73069\n");
            return 0;
        } else if (strcmp(argv[i], "list") == 0) 
        {
            found_list = 1;
            i++;

            while (i < argc) 
            {
                if (strcmp(argv[i], "recursive") == 0) 
                {
                    recursive = 1;
                } else if (strncmp(argv[i], "size_greater=", 13) == 0) 
                {
                    filter_size_greater = atoi(&argv[i][13]);
                } else if (strcmp(argv[i], "has_perm_write") == 0) 
                {
                    has_perm_write = 1;
                } else if (strncmp(argv[i], "path=", 5) == 0) 
                {
                    dir_path = argv[i] + 5;
                }
                i++;
            }
        } else if (strcmp(argv[i], "parse") == 0) 
        {
            i++;
            if (strncmp(argv[i], "path=", 5) == 0) 
            {
                char *file_path = argv[i] + 5;
                parse(file_path);
                return 0;
            } else 
            {
                printf("USAGE: ./a1 parse path=<file_path>\n");
                return 1;
            }
        } else {
            printf("USAGE: ./a1 list [recursive] <filtering_options> path=<dir_path>\n");
            return 1;
        }
    }

    if (!found_list) 
    {
        printf("USAGE: ./a1 list [recursive] <filtering_options> path=<dir_path>\n");
        return 1;
    }

    if (dir_path == NULL) 
    {
        perror("ERROR: no directory path specified");
        return 1;
    }

    if (recursive == 0) 
    {
        list(dir_path, filter_size_greater, has_perm_write);
    }

    if (recursive == 1)
    {
        printf("SUCCESS\n");
        list_recursive(dir_path, filter_size_greater, has_perm_write, dir_path);
        return 0;
    }
}
