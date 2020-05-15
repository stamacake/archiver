#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

char *cat(char *path, const char *fname)
{
	int length = strlen(path) + strlen(fname) + 1;
	char *buf = calloc(length + 1, 1);
	// chech + write
	assert(sprintf(buf, "%s/%s", path, fname) == length);
	return buf;
}

void arch(char *root, char *path, int fin_dir)
{
	DIR *dir;

	dir = opendir(path);
	const struct dirent *entry;
	/*
	struct dirent {
	ino_t          d_ino;       /* inode number */
  //off_t          d_off;       /* offset to the next dirent */
  //unsigned short d_reclen;    /* length of this record */
  //unsigned char  d_type;      /* type of file; not supported
  //                               by all file system types */
  // char           d_name[256]; /* filename */
 //	};

	while (entry = readdir(dir)) {
		
		if (entry->d_type != 4) { // chech is folder
			char *new = cat(path, entry->d_name); //full file name

			int len = strlen(new) - strlen(root); // only file name length

			assert(write(fin_dir, &len, sizeof(int)) != -1);// size of name
			assert(write(fin_dir, new + strlen(root),len) != -1); 
			
			int file_open = open(new, O_RDONLY);
			int size_f = lseek(file_open, 0, SEEK_END); // file size

			assert(write(fin_dir, &size_f, sizeof(int)) != -1);
			lseek(file_open, 0, SEEK_SET);
			void *buf = malloc(size_f);

			read(file_open, buf, size_f);
			assert(write(fin_dir, buf, size_f) != -1);
			free(buf);
			free(new);
			close(file_open);
		} else {
			//  directory points the current directory itself 
			//  and the ".." directory points to the parent directory.
			if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0)
				continue;
			char *new = cat(path, entry->d_name);

			arch(root, new, fin_dir);
			free(new);
		}
	}
	closedir(dir);
}

void make_dirs(char *dir, mode_t mode)
{
	size_t len = strlen(dir);
	char *dir_temp = calloc(len + 1, 1);

	strcpy(dir_temp, dir);
	for (size_t i = 0; i <= len; i++) {
		if (dir_temp[i] == '/') {
			dir_temp[i] = 0;
			mkdir(dir_temp, mode);
			dir_temp[i] = '/';
		}
	}
	mkdir(dir_temp, mode);
	free(dir_temp);
}

int create_path(char *path, mode_t mode)
{
	char *dir = calloc(strlen(path) + 1, 1);

	strcpy(dir, path);
	dirname(dir);
	make_dirs(dir, mode);
	free(dir);
	return creat(path, mode);
}

void unpack(char *path, int fin_dir){
	int buf;

	while (read(fin_dir, &buf, sizeof(int)) == sizeof(int)) {
		char *path_entry = calloc(buf + 1, 1);

		read(fin_dir, path_entry, buf); // name
		int len_data;

		read(fin_dir, &len_data, sizeof(int));
		void *data = malloc(len_data);

		read(fin_dir, data, len_data);
		char *path_full = cat(path, path_entry);

		free(path_entry);
		int file_dir = create_path(path_full, 0700);

		write(file_dir, data, len_data);
		close(file_dir);
		free(path_full);
		free(data);
	}
}


void show_help(){
	puts("-p, -pack from_root where_root \n");
	puts("-up, -unpack from_root where_root \n");	
}

int main(int argc, char *argv[])
{
	
	
	// a.out // command // from // where
	if (argc != 4) {
		show_help();
	} else {
		if (strcmp(argv[1], "-pack") == 0 || strcmp(argv[1], "-p") == 0) {
			
			int fin_dir = creat(argv[3], 0700); // final file
			// 0700 permission mask
			arch(argv[2], argv[2], fin_dir); //pack
			
			close(fin_dir);
			
		} else if (strcmp(argv[1], "unpack") == 0 || strcmp(argv[1], "-up") == 0) {
			char *from = argv[2];
			int fin_dir = open(from, O_RDONLY);

			unpack(argv[3], fin_dir);
			close(fin_dir);
		} else {
			show_help();
		}
	}
	return 0;
}
