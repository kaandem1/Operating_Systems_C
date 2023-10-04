#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define REQUEST_PIPE "REQ_PIPE_73069"
#define RESPONSE_PIPE "RESP_PIPE_73069"
#define SHM_NAME "/nJJaZTR4"
#define BUFFER_SIZE 1024
#define VARIANT_REQUEST "VARIANT"
#define CREATE_SHARED_MEMORY_REQUEST "CREATE_SHM"
#define WRITE_TO_SHM_REQUEST "WRITE_TO_SHM"
#define MAP_FILE_REQUEST "MAP_FILE"
#define READ_FROM_FILE_OFFSET_REQUEST "READ_FROM_FILE_OFFSET"
#define READ_FROM_FILE_SECTION_REQUEST "READ_FROM_FILE_SECTION"
#define READ_FROM_LOGICAL_SPACE_OFFSET_REQUEST "READ_FROM_LOGICAL_SPACE_OFFSET"
#define EXIT_REQUEST "EXIT"
#define NEXT_AVAILABLE_MULTIPLE 5120
#define START "START!"
#define END_CHARACTER '!'

void *shm_ptr = NULL;
void *f_ptr = NULL;
unsigned int shm_size = 0;
unsigned int f_size = 0;

void respond(int resp_p, const char *resp_1, const char *resp_2)
{
    write(resp_p, resp_1, strlen(resp_1));
    write(resp_p, resp_2, strlen(resp_2));
}

void get_f_name(int req_p, char* f_name) {
    int i = 0;
    char c;
    while (read(req_p, &c, 1) > 0 && c != END_CHARACTER) {
        f_name[i++] = c;
    }
    f_name[i] = '\0';
}

void read_input(int req_p, int* section_no, int* offset, int* no_of_bytes) {
    read(req_p, section_no, sizeof(*section_no));
    read(req_p, offset, sizeof(*offset));
    read(req_p, no_of_bytes, sizeof(*no_of_bytes));
}

void goto_sect_start(void** s_start, int section_no) {
    *s_start -= 2;
    short h_size;
    memcpy(&h_size, *s_start, sizeof(h_size));
    *s_start -= h_size;
    *s_start = f_ptr + (f_size - h_size + 5);
    *s_start += ((section_no - 1) * 26);
}

void section_info(void* s_start, int* s_offset, int* s_size) {
    memcpy(s_size, s_start + 18 + sizeof(*s_offset), sizeof(*s_size));
    memcpy(s_offset, s_start + 18, sizeof(*s_offset));
}

void copy_section(void* shm_ptr, void* f_ptr, int s_offset, int offset, int no_of_bytes) {
    memcpy(shm_ptr, f_ptr + s_offset + offset, no_of_bytes);
}

void goto_h_start(void** h_start) {
    *h_start -= 2;
    short h_size;
    memcpy(&h_size, *h_start, sizeof(h_size));
    *h_start -= h_size;
    *h_start = f_ptr + (f_size - h_size + 5);
}

void section_info_h(void* h_start, int* s_offset, int* s_size) {
    memcpy(s_size, h_start + 18 + sizeof(*s_offset), sizeof(*s_size));
    memcpy(s_offset, h_start + 18, sizeof(*s_offset));
}

void copy_logical_s(void* shm_ptr, void* f_ptr, int s_offset, int logical_space, int logical_offset, int no_of_bytes) {
    memcpy(shm_ptr, f_ptr + s_offset + (logical_offset - logical_space), no_of_bytes);
}



void variant(int resp_p)
{
    const char *resp_1 = "VARIANT!";
    write(resp_p, resp_1, strlen(resp_1));
    const char *resp_2 = "VALUE!";
    write(resp_p, resp_2, strlen(resp_2));
    unsigned int variant_num = 73069;
    write(resp_p, &variant_num, sizeof(variant_num));
}

void create_shm(int req_p, int resp_p)
{
    unsigned int size_shm;
    read(req_p, &size_shm, sizeof(size_shm));

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0664);
    if (shm_fd == -1 || ftruncate(shm_fd, size_shm) == -1)
    {
        respond(resp_p, "CREATE_SHM!", "SUCCESS!");
        close(shm_fd);
    }
    else
    {
        respond(resp_p, "CREATE_SHM!", "SUCCESS!");

        shm_ptr = mmap(0, size_shm, PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shm_ptr == MAP_FAILED)
        {
            perror("Map failed");
            return;
        }

        close(shm_fd);
        shm_size = size_shm;
    }
}

void write_to_shm(int req_p, int resp_p)
{
    unsigned int shm_offset;
    unsigned int value;
    read(req_p, &shm_offset, sizeof(shm_offset));
    read(req_p, &value, sizeof(value));

    if (shm_offset + sizeof(value) > shm_size)
    {
        respond(resp_p, "WRITE_TO_SHM!", "ERROR!");
    }
    else
    {
        *((unsigned int *)(shm_ptr + shm_offset)) = value;
        respond(resp_p, "WRITE_TO_SHM!", "SUCCESS!");
    }
}

void map_file(int req_p, int resp_p)
{
    char f_name[256];
    get_f_name(req_p, f_name);
    
    int f_fd = open(f_name, O_RDONLY);
    if (f_fd == -1)
    {
        respond(resp_p, "MAP_FILE!", "ERROR!");
    }
    else
    {
        struct stat st;
        fstat(f_fd, &st);
        f_size = st.st_size;
        f_ptr = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, f_fd, 0);

        if (f_ptr == MAP_FAILED)
        {
            respond(resp_p, "MAP_FILE!", "ERROR!");
        }
        else
        {
            respond(resp_p, "MAP_FILE!", "SUCCESS!");
        }

        close(f_fd);
    }
}

void read_file_offset(int req_p, int resp_p)
{
    unsigned int offset, no_of_bytes;
    read(req_p, &offset, sizeof(offset));
    read(req_p, &no_of_bytes, sizeof(no_of_bytes));

    if (shm_ptr == NULL || f_ptr == NULL || offset + no_of_bytes > f_size || no_of_bytes > shm_size)
    {
        respond(resp_p, "READ_FROM_FILE_OFFSET!", "ERROR!");
    }
    else
    {
        memcpy(shm_ptr, f_ptr + offset, no_of_bytes);
        respond(resp_p, "READ_FROM_FILE_OFFSET!", "SUCCESS!");
    }
}


void read_file_section(int req_p, int resp_p)
{
    int section_no, offset, no_of_bytes;
    read_input(req_p, &section_no, &offset, &no_of_bytes);

    if (shm_ptr == NULL || f_ptr == NULL || no_of_bytes > shm_size)
    {
        respond(resp_p, "READ_FROM_FILE_SECTION!", "ERROR!");
        return;
    }

    void *s_start = f_ptr + f_size - 1;
    goto_sect_start(&s_start, section_no);

    int s_offset, s_size;
    section_info(s_start, &s_offset, &s_size);

    if (offset + no_of_bytes >= s_size)
    {
        respond(resp_p, "READ_FROM_FILE_SECTION!", "ERROR!");
    }
    else
    {
        copy_section(shm_ptr, f_ptr, s_offset, offset, no_of_bytes);
        respond(resp_p, "READ_FROM_FILE_SECTION!", "SUCCESS!");
    }
}

void read_logical_space(int req_p, int resp_p)
{
    unsigned int logical_offset, no_of_bytes;
    read(req_p, &logical_offset, sizeof(logical_offset));
    read(req_p, &no_of_bytes, sizeof(no_of_bytes));

    if (shm_ptr == NULL || f_ptr == NULL || no_of_bytes > shm_size)
    {
        respond(resp_p, "READ_FROM_LOGICAL_SPACE_OFFSET!", "ERROR!");
        return;
    }

    int logical_space = 0;
    void *h_start = f_ptr + f_size - 1;
    goto_h_start(&h_start);

    while (logical_space <= logical_offset)
    {
        int s_offset, s_size;
        section_info_h(h_start, &s_offset, &s_size);

        if (logical_space + s_size > logical_offset && logical_space + s_size >= logical_offset + no_of_bytes)
        {
            copy_logical_s(shm_ptr, f_ptr, s_offset, logical_space, logical_offset, no_of_bytes);
            respond(resp_p, "READ_FROM_LOGICAL_SPACE_OFFSET!", "SUCCESS!");
            return;
        }

        logical_space += ((s_size + NEXT_AVAILABLE_MULTIPLE - 1) / NEXT_AVAILABLE_MULTIPLE) * NEXT_AVAILABLE_MULTIPLE;
        h_start += 26;
    }

    respond(resp_p, "READ_FROM_LOGICAL_SPACE_OFFSET!", "ERROR!");
}


void req_proc(char *buffer, int req_p, int resp_p)
{
    if (strcmp(buffer, VARIANT_REQUEST) == 0)
    {
        variant(resp_p);
    }
    else if (strcmp(buffer, CREATE_SHARED_MEMORY_REQUEST) == 0)
    {
        create_shm(req_p, resp_p);
    }
    else if (strcmp(buffer, WRITE_TO_SHM_REQUEST) == 0)
    {
        write_to_shm(req_p, resp_p);
    }
    else if (strcmp(buffer, MAP_FILE_REQUEST) == 0)
    {
        map_file(req_p, resp_p);
    }
    else if (strcmp(buffer, READ_FROM_FILE_OFFSET_REQUEST) == 0)
    {
        read_file_offset(req_p, resp_p);
    }
    else if (strcmp(buffer, READ_FROM_FILE_SECTION_REQUEST) == 0)
    {
        read_file_section(req_p, resp_p);
    }
    else if (strcmp(buffer, READ_FROM_LOGICAL_SPACE_OFFSET_REQUEST) == 0)
    {
        read_logical_space(req_p, resp_p);
    }
}

void pipe_init(int *req_p, int *resp_p)
{
    unlink(RESPONSE_PIPE);

    if (mkfifo(RESPONSE_PIPE, 0666) == -1)
    {
        perror("ERROR\ncannot create the response pipe");
        exit(1);
    }
    if ((*req_p = open(REQUEST_PIPE, O_RDONLY)) == -1)
    {
        perror("ERROR\ncannot open the request pipe");
        unlink(RESPONSE_PIPE);
        exit(1);
    }
    if ((*resp_p = open(RESPONSE_PIPE, O_WRONLY)) == -1)
    {
        perror("ERROR\ncannot open the response pipe");
        close(*req_p);
        unlink(RESPONSE_PIPE);
        exit(1);
    }

    write(*resp_p, START, strlen(START));
    printf("SUCCESS\n");
}

void clean_up(int req_p, int resp_p)
{
    close(req_p);
    close(resp_p);
    unlink(RESPONSE_PIPE);
}

void exit_req(int req_p, int resp_p)
{
    close(req_p);
    close(resp_p);
    unlink(RESPONSE_PIPE);
}

void requests(int req_p, int resp_p)
{
    char buffer[BUFFER_SIZE];
    int size = 0;

    while (1)
    {
        char c;
        while (read(req_p, &c, 1) > 0 && c != END_CHARACTER)
        {
            buffer[size++] = c;
        }
        buffer[size] = '\0';

        if (strcmp(buffer, EXIT_REQUEST) == 0)
        {
            exit_req(req_p, resp_p);
            break;
        }

        req_proc(buffer, req_p, resp_p);

        size = 0;
    }
}

int main()
{
    int req_p, resp_p;

    pipe_init(&req_p, &resp_p);

    requests(req_p, resp_p);

    clean_up(req_p, resp_p);

    return 0;
}
