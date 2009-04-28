#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#define	DISK_SIZE	(2880 * 512)
char buffer[1024];
int main(int argc, char **argv)
{
	int fd_in;
	int fd_out;
	struct stat fin_stat;

	size_t file_size;
	size_t read_size;
	size_t remain_size;

	if (argc < 3)
		return -1;

	fd_in = open(argv[1], O_RDONLY);
	if (fd_in < 0) {
		printf("Can not open %s \n", argv[1]);
		return -1;
	}

	if (fstat(fd_in, &fin_stat) < 0) {
		printf("Can not stat %s \n", argv[1]);
		return -1;
	}

	if (fin_stat.st_size > 512) {
		printf("loader file is too large \n");
		return -1;
	}

	fd_out = open("./boot.img", O_CREAT | O_RDWR, S_IRWXU);
	if (fd_out < 0) {
		perror("Can not create output file");
		return -1;
	}

	if (read(fd_in, buffer, 512) <= 0) {
		printf("Can not read loader file");
		return -1;
	}

	if (write(fd_out, buffer, 512) < 0) {
		perror("Can not write output file");
		return -1;
	}

	close(fd_in);
#if 0
	close(fd_out);
	return 0;
#endif
	fd_in = open(argv[2], 0, O_RDONLY);
	if (fstat(fd_in, &fin_stat) < 0) {
		printf("Can not stat %s \n", argv[2]);
		return -1;
	}

	file_size = fin_stat.st_size;
	while (file_size) {
		if ((read_size = read(fd_in, buffer, sizeof(buffer))) < 0) {
			perror("Can not read loader file");
			return -1;
		}

		file_size -= read_size;
		if (write(fd_out, buffer, read_size) < 0) {
			perror("Can not write output file");
			return -1;
		}
	}

	memset(buffer, 0, sizeof(buffer));
	remain_size = DISK_SIZE - 512 - fin_stat.st_size;
	while (remain_size) {
		size_t size;

		size = remain_size < sizeof(buffer) ? remain_size : sizeof(buffer);
		if (write(fd_out, buffer, size) < 0) {
			perror("Can not write output file");
			return -1;
		}

		remain_size -= size;
	}

	close(fd_in);
	close(fd_out);
	return 0;
}
