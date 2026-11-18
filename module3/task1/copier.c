#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BLOCK_SIZE 4096   // содержимое файлов передаёт по 4 КБ
#define NAME_SIZE 256     // под имя файла в заголовке

// первое сообщение о каждом файле (пустое поле name - "файлов больше нет")
typedef struct {
    char name[NAME_SIZE];
    long size;            // сколько байт содержимого пойдёт следом
} FileHeader;


// читаем, пока не наберётся size байт (или пока канал не закроется)
static ssize_t read_full(int fd, void *buf, size_t size)
{
    size_t done = 0;
    while (done < size) {
        ssize_t n = read(fd, (char *)buf + done, size - done);
        if (n > 0) {
            done += (size_t)n;
        } else if (n == 0) {
            break;                    // данных больше нет
        } else if (errno != EINTR) {
            return -1;                // ошибка
        }                             // EINTR — прервали сигналом, повторяем
    }
    return (ssize_t)done;
}

// пишем, пока всё не уйдёт
static ssize_t write_full(int fd, const void *buf, size_t size)
{
    size_t done = 0;
    while (done < size) {
        ssize_t n = write(fd, (const char *)buf + done, size - done);
        if (n > 0) {
            done += (size_t)n;
        } else if (n < 0 && errno != EINTR) {
            return -1;
        }
    }
    return (ssize_t)done;
}


// отправка одного файла ребёнку
static void send_file(int out_fd, const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        // диагностика в stderr, работа продолжается
        fprintf(stderr, "parent: не могу открыть %s: %s\n", path, strerror(errno));
        return;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        fprintf(stderr, "parent: не могу узнать размер %s: %s\n", path, strerror(errno));
        close(fd);
        return;
    }
    if (strlen(path) >= NAME_SIZE) {
        fprintf(stderr, "parent: слишком длинное имя файла: %s\n", path);
        close(fd);
        return;
    }

    // собщение 1: заголовок с именем и размером
    FileHeader header;
    memset(&header, 0, sizeof(header));  // нули - "\0" в конце имени
    strcpy(header.name, path);
    header.size = st.st_size;

    if (write_full(out_fd, &header, sizeof(header)) != (ssize_t)sizeof(header)) {
        fprintf(stderr, "parent: не удалось отправить заголовок\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // сообщение 2: содержимое, блоками по BLOCK_SIZE байт
    long remaining = header.size;
    char buffer[BLOCK_SIZE];
    while (remaining > 0) {
        size_t chunk = remaining < BLOCK_SIZE ? (size_t)remaining : BLOCK_SIZE;
        if (read_full(fd, buffer, chunk) != (ssize_t)chunk ||
            write_full(out_fd, buffer, chunk) != (ssize_t)chunk) {
            fprintf(stderr, "parent: ошибка при передаче %s\n", path);
            close(fd);
            exit(EXIT_FAILURE);
        }
        remaining -= (long)chunk;
    }

    close(fd);
    printf("parent: отправил %s (%ld байт)\n", path, header.size);
}


// цикл принёма файлов и сохранения копий
static void child_loop(int in_fd, int out_fd)
{
    // сообщаем о готовности к приёму
    char ready = 'R';
    if (write_full(out_fd, &ready, 1) != 1) {
        fprintf(stderr, "child: не удалось отправить готовность\n");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        FileHeader header;
        if (read_full(in_fd, &header, sizeof(header)) != (ssize_t)sizeof(header)) {
            fprintf(stderr, "child: канал закрылся раньше времени\n");
            exit(EXIT_FAILURE);
        }

        if (header.name[0] == '\0') {
            printf("child: получена команда завершения\n");
            return;
        }

        char copy_name[NAME_SIZE + 5];   // запас на ".copy" и "\0"
        snprintf(copy_name, sizeof(copy_name), "%s.copy", header.name);

        int copy_fd = open(copy_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (copy_fd == -1) {
            fprintf(stderr, "child: не могу создать %s: %s\n",
                    copy_name, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // ровно header.size байт блоками
        long remaining = header.size;
        char buffer[BLOCK_SIZE];
        while (remaining > 0) {
            size_t chunk = remaining < BLOCK_SIZE ? (size_t)remaining : BLOCK_SIZE;
            if (read_full(in_fd, buffer, chunk) != (ssize_t)chunk ||
                write_full(copy_fd, buffer, chunk) != (ssize_t)chunk) {
                fprintf(stderr, "child: ошибка при передаче %s\n", header.name);
                close(copy_fd);
                exit(EXIT_FAILURE);
            }
            remaining -= (long)chunk;
        }

        close(copy_fd);
        printf("child: сохранил %s (%ld байт)\n", copy_name, header.size);
    }
}


int main(int argc, char **argv)
{
    // -p <имя> = именованный канал, остальное — файлы ---
    const char *fifo_name = NULL;
    const char **files = calloc((size_t)argc, sizeof(*files));
    int file_count = 0;

    if (files == NULL) {
        perror("calloc");
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "После -p нужно указать имя канала\n");
                return EXIT_FAILURE;
            }
            fifo_name = argv[++i];
        } else {
            files[file_count++] = argv[i];
        }
    }

    if (file_count == 0) {
        fprintf(stderr, "Использование: %s [-p имя_канала] файл1 [файл2 ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // подготовка каналов: data - заголовки и содержимое, ready - сообщение о готовности
    int data_pipe[2], ready_pipe[2];
    char ready_fifo[PATH_MAX];

    if (fifo_name == NULL) {
        if (pipe(data_pipe) == -1 || pipe(ready_pipe) == -1) {
            perror("pipe");
            return EXIT_FAILURE;
        }
    } else {
        snprintf(ready_fifo, sizeof(ready_fifo), "%s.ready", fifo_name);
        if (mkfifo(fifo_name, 0666) == -1 || mkfifo(ready_fifo, 0666) == -1) {
            perror("mkfifo");
            return EXIT_FAILURE;
        }
    }

    // создаём дочерний процесс
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // дочерний процесс
        int in_fd, out_fd;
        if (fifo_name == NULL) {
            close(data_pipe[1]);   // ребёнок не пишет в data и не читает из ready
            close(ready_pipe[0]);  // ненужное закрываем
            in_fd = data_pipe[0];
            out_fd = ready_pipe[1];
        } else {
            // канал открывается по имени
            in_fd = open(fifo_name, O_RDONLY);
            out_fd = open(ready_fifo, O_WRONLY);
            if (in_fd == -1 || out_fd == -1) {
                perror("open fifo");
                _exit(EXIT_FAILURE);
            }
        }
        child_loop(in_fd, out_fd);
        close(in_fd);
        close(out_fd);
        exit(EXIT_SUCCESS);
    }

    // родительский процесс
    int in_fd, out_fd;
    if (fifo_name == NULL) {
        close(data_pipe[0]);
        close(ready_pipe[1]);
        out_fd = data_pipe[1];
        in_fd = ready_pipe[0];
    } else {
        out_fd = open(fifo_name, O_WRONLY);
        in_fd = open(ready_fifo, O_RDONLY);
        if (in_fd == -1 || out_fd == -1) {
            perror("open fifo");
            return EXIT_FAILURE;
        }
    }

    // ждём сообщения о готовности (ребёнок пишет первым)
    char ready;
    if (read_full(in_fd, &ready, 1) != 1) {
        fprintf(stderr, "parent: ребёнок не сообщил о готовности\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < file_count; i++)
        send_file(out_fd, files[i]);

    // все файлы отправлены (заголовок с пустым именем - команда завершения)
    FileHeader end;
    memset(&end, 0, sizeof(end));
    write_full(out_fd, &end, sizeof(end));

    close(out_fd);
    close(in_fd);

    // дожидаемся завершения дочернего процесса
    int status;
    waitpid(pid, &status, 0);

    if (fifo_name != NULL) {       // именованные каналы — файлы
        unlink(fifo_name);
        unlink(ready_fifo);
    }

    free(files);
    printf("parent: работа завершена\n");
    return EXIT_SUCCESS;
}