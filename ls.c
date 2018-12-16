#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>


void dirwalk(char *name);
void printinfaboutfile(char *name);

int flag_a = 0;
int flag_r = 0;
int flag_l = 0;
struct dirent *dp;

void dirwalk(char *name ){
	DIR *dfd = opendir(name);//opendir возвращает указатель на структуру типа DIR с информацией о каталоге
	//struct dirent *dp;
	struct stat aboutfile;
	char newname[100];
	
	while ((dp = readdir(dfd)) != NULL){//readdir возвращает следующего файла в каталоге
		if ((((!strcmp(dp -> d_name, ".") || !strcmp(dp -> d_name, "..")) && (!flag_a))))//если нам не нужно выводить скрытые файлы
			 continue;//пропускаем информацию этой папки и папки ее отца,чтобы не зациклиться

		sprintf(newname, "%s/%s", name, dp -> d_name);
		if (flag_r){
			lstat(newname, &aboutfile);
		if((aboutfile.st_mode & S_IFMT) == S_IFDIR)
			if((strcmp(dp->d_name,"..")) && strcmp(dp->d_name, ".")) dirwalk(newname);
		printinfaboutfile(newname);//печать инфы об обычном файле	
		}else
		printinfaboutfile(newname);
	}
	closedir(dfd);
}

void printinfo(char *name){
	
	struct stat aboutfile;
	lstat (name, &aboutfile);
	if((aboutfile.st_mode & S_IFMT) == S_IFDIR){//если файл - каталог
				dirwalk(name);
			}else//если файл - файл
				printinfaboutfile(name);//печать информации о файле
}

void printinfaboutfile(char* name){
	
	struct stat aboutfile;
	if ((flag_a) && (!flag_l)){
		printf(" %s \n", name);
	}
	lstat (name, &aboutfile); //считали информацию о name 
	if (flag_l){
		if ((aboutfile.st_mode & S_IFMT) == S_IFDIR){ //каталог
			printf("d"); 
		}
		if ((aboutfile.st_mode & S_IFMT) == S_IFCHR){ //символьное устройство
			printf("c"); 
		}
		if ((aboutfile.st_mode & S_IFMT) == S_IFBLK){ //файл блочного устройства
			printf("b"); 
		}
		if ((aboutfile.st_mode & S_IFMT) == S_IFREG){ //если обычный файл
			printf("-");
		}
		if ((aboutfile.st_mode & S_IFMT) == S_IFLNK) { //символьная ссылка
			printf("l");
		}
		if ((aboutfile.st_mode & S_IFMT) == S_IFIFO) { //FIFO именованный канал
			printf("p");
		}
		if ((aboutfile.st_mode & S_IFMT) == S_IFSOCK) { //сокет - доменное гнезд
			printf("s");
		}
		
		if (aboutfile.st_mode & S_IRUSR){
			printf ("r");
		}else{
			printf("-");
		}
		if (aboutfile.st_mode & S_IWUSR){
			printf ("w");
		}else{
			printf("-");
		}
		if (aboutfile.st_mode & S_IXUSR){//права на выполнение
			printf ("x");
		}else{
			printf("-");
		}
		if (aboutfile.st_mode & S_IRGRP){
			printf ("r");
		}else{
			printf("-");
		}
		if (aboutfile.st_mode & S_IWGRP){
			printf ("w");
		}else{
			printf("-");
		}
		if (aboutfile.st_mode & S_IXGRP){//права на выполнение
			printf ("x");
		}else{
			printf("-");
		}
		//права всех остальных
		if (aboutfile.st_mode & S_IROTH){
			printf ("r");
		}else{
			printf("-");
		}
		if (aboutfile.st_mode & S_IWOTH){
			printf ("w");
		}else{
			printf("-");
		}
		if (aboutfile.st_mode & S_IXOTH){//права на выполнение
			printf ("x");
		}else{
			printf("-");
		}
		
		struct passwd *username = getpwuid(aboutfile.st_uid); // считываем id владельца, который можно найти в структуре stat, 
														//getwuid получает информацию о владельце с помощью его  id											
		struct group *aboutgroup = getgrgid(aboutfile.st_gid); //getgrgid возвращает указатель на структуру, в которой мы можем узнать информацию о группе 		
		printf(" %hu", aboutfile.st_nlink); //number of links
		printf(" %s", username->pw_name);// username
		printf(" %s", aboutgroup->gr_name);//groupname
		printf(" %lld", aboutfile.st_size); //size
		
		//вывод даты и времени
		struct tm *time;
		char strtime[16];
		time = localtime(&aboutfile.st_atime);// преобразует в строуктуру возвращает указатель на структуру, содержащую преобразованное 
						   //системное время в дату и местное время
		strftime(strtime, 16, " %b  %d  %H:%M", time); //переводит в строку заданного формата
		printf(" %s", strtime);	   
		printf(" %s \n", name);
		
							
	}
	
}

int main(int argc, char **argv)

{
	char *name;
	
	for(int i = 1; i < argc -1; i++){
		if(strstr(argv[i], "-a")){
			flag_a = 1;
		} 
		if (strstr(argv[i], "-R")){
			flag_r = 1;
		} 
		if (strstr(argv[i], "-l")){
			flag_l = 1;
		}
		if ((flag_a == 0) && (flag_r == 0) && (flag_l == 0)){
			printf("LS doesn't understand your command\n");
		}
	}
	name = argv[argc - 1];
	printinfo(name);
	return 0;
}

/*
 * typedef struct{ 
 * int fd;         файловый дескриптор каталога
 * dirent d;       пункт списка файлов
 * } DIR; 
 * 
 * typedef struct{ 
 * long d_ino;               номер индекса
 * char d_name[NAME_MAX+1]; имя +\0
 *      d_off 
 * }Dirent;
 * 
 * struct stat {    информация о файле
 * dev_t st_dev     номер индекса
 * ino_t st_ino  	биты режима
 * short st_mode 	
 * unsigned	short st_nlink	количество ссылок на файл
 * short st_uid 	id 	владельца
 * short st_gid 	id группы владельца
 * dev_t st_rdev 
 * off_t st_size 	размер файла в символах
 * time_t st_atime 	время последнего обращения
   time_t st_mtime 	время последней модификации
   time_t st_ctime 	время последней модификации индекса
 * };
 *    struct passwd {
// 	char    *pw_name;        user name 
 //	char    *pw_passwd;      user password 
 //	uid_t   pw_uid;          user id 
//	gid_t   pw_gid;          group id 
//	char    *pw_gecos;      real name 
//	char    *pw_dir;         home directory 
//	char    *pw_shell;       shell program 

//struct group {
 //       char    *gr_name;         название группы 
 //       char    *gr_passwd;       пароль группы 
//        gid_t   gr_gid;           идентификатор группы 
//        char    **gr_mem;         члены группы 
* 
* struct tm{//
* int tm_sec – секунды (отсчет с 0); 
int tm_min – минуты (отсчет с 0); 
int tm_hour - часы (отсчет с 0); 
int tm_mday - день месяца (отсчет с 1); 
int tm_mon - месяц (отсчет с 0); 
int tm_year – год (за начала отсчета принят 1900 год); 
int tm_wday - день недели (воскресенье - 0); 
int tm_yday - день в году (отсчет с 0); 
int tm_isdst - признак "летнее время" (больше нуля если «летнее время», ноль если «зимнее время», меньше нуля если нет информации. 

* };
* 
* sys/stat.h:

#define S_IFMT  0xF000           type of file 
#define S_IFIFO 0x1000  fifo 
#define S_IFCHR 0x2000   character special 
#define S_IFDIR 0x4000  directory 
#define S_IFBLK 0x6000  block special
#define S_IFREG 0x8000  regular
#define S_IFLNK 0xA000  symbolic link
#define S_IFNAM 0x5000  special named file
#define S_INSEM 0x1    XENIX semaphore subtype of IFNAM file 
#define S_INSHD 0x2      XENIX shared data subtype of IFNAM file
#define S_ISUID 04000    set user id on execution 
#define S_ISGID 02000    set group id on execution 
#define S_ISVTX 01000    save swapped text even after use 
#define S_IREAD 00400    read permission, owner 
#define S_IWRITE 00200   write permission, owner 
#define S_IEXEC 00100    execute/search permission, owner 
#define S_ENFMT S_ISGID record locking enforcement flag 
#define S_IRWXU 00700    read, write, execute: owner 
#define S_IRUSR 00400    read permission: owner 
#define S_IWUSR 00200    write permission: owner 
#define S_IXUSR 00100   execute permission: owner 
#define S_IRWXG 00070    read, write, execute: group 
#define S_IRGRP 00040    read permission: group 
#define S_IWGRP 00020    write permission: group 
#define S_IXGRP 00010    execute permission: group 
#define S_IRWXO 00007    read, write, execute: other 
#define S_IROTH 00004    read permission: other 
#define S_IWOTH 00002    write permission: other 
#define S_IXOTH 00001    execute permission: other 
* 
*/
