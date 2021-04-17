using namespace std;

#include <stdlib.h>  
#include <stdio.h> 
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>

#include <sched.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <sys/fanotify.h>
#include <sys/epoll.h>
#include <list>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include <iostream>
#include <stdio.h>
#include <string.h>

int process_check();
int process_record();
int init_daemon(char* path);
bool create_fanotify(int& fd);
bool add_watch_unit(const string& file_path);
bool open_fanotify();
bool fanotify_handle_event(const int& event_count);
bool fanotify_decision_event(const struct fanotify_event_metadata* metadata, const string& path);
bool start_process_hidden(int argc, char* argv[]);
bool stop_process_hidden();
string get_proc_name(const pid_t& pid);

int fanotify_fd;
bool run_flag;
pid_t pid;

#define process_hidden_pid "/tmp/xxyc.pid"

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		cout << "./xxyc pid pid ..." << endl;
		cout << "./xxyc stop" << endl;
		exit(1);
	}
	else if (argc == 2)
	{
		string argv_temp = argv[1];
		if (argv_temp == "stop")
		{
			process_check();
			cout << "stop success" << endl;
			exit(1);
		}
	}
	else
	{
		process_check();
	}

	init_daemon("/tmp");
	pid = getpid();
	cout << pid << endl;

	process_record();

	fanotify_fd = 0;
	run_flag = true;
	pid = getpid();
	cout << pid << endl;

	create_fanotify(fanotify_fd);
	start_process_hidden(argc, argv);
	open_fanotify();
}

int process_check()
{
	int fd;
	if ((fd = open(process_hidden_pid, O_CREAT | O_RDONLY, 0600)) < 0) {
		perror("open");
		exit(1);
	}
	char pid_temp[20];
	char file_path_temp[200];
	int n = read(fd, pid_temp, 20);
	pid_t childpid = atoi(pid_temp);
	cout << childpid << endl;
	snprintf(file_path_temp, sizeof(file_path_temp), "/proc/%d", childpid);
	if (access(file_path_temp, F_OK) != 0) //����ļ�������
	{
		cout << file_path_temp << " is not exists, mesg=" << strerror(errno) << endl;
	}
	else
	{
		int retval = kill(childpid, SIGKILL);
		if (retval)
		{
			perror("kill");
			exit(1);
		}
	}
	close(fd);
}

int process_record()
{
	int fd;
	if ((fd = open(process_hidden_pid, O_CREAT | O_WRONLY | O_TRUNC, 0600)) < 0) {
		perror("open");
		exit(1);
	}
	stringstream pid_ss;
	string pid_str;
	pid_ss << getpid();
	pid_ss >> pid_str;
	write(fd, pid_str.c_str(), pid_str.length());//д���ļ�
	close(fd);
}

int init_daemon(char* path)
{
	int pid;
	if ((pid = fork()) > 0)
	{
		exit(0); /* �Ǹ����̣����������� */
	}
	else if (pid < 0)
	{
		return -1; /* forkʧ�� */
	}

	/* �ǵ�һ�ӽ��̣����� */
	/* ����ý�����һ����������鳤���˺������ش���
	Ϊ�˱�֤��һ�㣬�����ȵ���fork()Ȼ��exit()����ʱֻ���ӽ��������У�
	�ӽ��̼̳��˸����̵Ľ�����ID�����ǽ���PIDȴ���·���ģ����Բ�����
	���»Ự�Ľ������PID���Ӷ���֤����һ�㡣 */
	setsid();
	/* ����һ���µĻỰ */
	/* ��һ�ӽ��̳�Ϊ�µĻỰ�鳤�ͽ����鳤 */
	/* ��������նˣ���¼�Ự�ͽ����� */

	/* ���ڣ���һ�ӽ����Ѿ���Ϊ���ն˵ĻỰ�鳤�������������������һ��
	�����նˡ�    ����ͨ��ʹ���̲��ٳ�Ϊ�Ự�鳤����ֹ�������´򿪿����ն� */
	if ((pid = fork()) > 0)
	{
		exit(0); /* �ǵ�һ�ӽ��̣�������һ�ӽ��� */
	}
	else if (pid < 0)
	{
		return -1; /* forkʧ�� */
	}

	/* �ǵڶ��ӽ��̣����� */
	/* �ڶ��ӽ��̲����ǻỰ�鳤 */

	/* �ı乤��Ŀ¼��/tmp */
	/* ������л�����Ŀ¼����Ϊ������ĵ�ǰĿ¼����һ������װ��  �ļ�ϵͳ��,��ô�ͻ��������ļ�ϵͳ
	��ж�أ��Ǳ���,��ֹmount���� */
	if (chdir(path ? path : "/tmp") != 0)
		return -2;

	/* parasoft suppress item BD-RES-INVFREE-1 */
	/* �رձ�׼�����������׼������ļ������� */
	close(STDIN_FILENO); /* parasoft-suppress BD-RES-INVFREE-1 */
	close(STDOUT_FILENO); /* parasoft-suppress BD-RES-INVFREE-1 */
	close(STDERR_FILENO); /* parasoft-suppress BD-RES-INVFREE-1 */
	/* parasoft unsuppress item BD-RES-INVFREE-1 */

	umask(0);
	/* ���̴Ӵ������ĸ���������̳����ļ�������ģ���������޸��ػ��������������ļ�
	�Ĵ�ȡλ��umask���ð��ػ����̵�umask����Ϊ0������ȡ���˸����̵�umask��������Ǳ�ڵ�
	���Ŵ����ļ���Ŀ¼ */

	return 0;
}

bool create_fanotify(int& fd) //��������inotifyʵ��
{
	//��ʼ������һ������ΪFAN_CLASS_CONTENT����ʾ�������֪ͨ�ļ��¼�����һ��ֵΪFAN_CLASS_NOTIF ΪĬ��ֵ��ֻ��������¼�֪ͨ����һ��ļ����У�ʹ��FAN_CLASS_CONTENT
	//�ڶ�����������ʾ���յ������ļ��¼��󣬱�������ļ��Ĳ���Ϊ�ɶ���д����Ϊmetadata�е�fdΪ������������¼����ļ���fd������ֱ�Ӳ����Ǹ��ļ����������Ҫ�ص���
	fd = fanotify_init(FAN_CLASS_CONTENT, O_RDWR);
	if (fd < 0)
	{
		cout << "cteate fanotify fail, fd:" << fd << ", mesg=" << strerror(errno) << endl;
	}
	else
	{
		cout << "cteate fanotify success, fd:" << fd << endl;
	}
	return true;
}
bool add_watch_unit(const string& file_path) //��inotify�����һ����أ�ֻ���Ŀ¼��
{
	int ret = fanotify_mark(fanotify_fd, FAN_MARK_ADD, FAN_OPEN_PERM | FAN_ACCESS_PERM | FAN_EVENT_ON_CHILD | FAN_ONDIR, AT_FDCWD, file_path.c_str());
	if (ret < 0)
	{
		cout << file_path << " add fanotify fail, mesg=" << strerror(errno) << endl;
	}
	else
	{
		cout << file_path << " add fanotify success, ret:" << ret << endl;
	}
	return true;
}
bool open_fanotify() //��ʼ����
{
	struct epoll_event epev;  //��¼�׽��������Ϣ
	epev.events = EPOLLIN;    //���������ݿɶ��¼�
	epev.data.fd = fanotify_fd;//�ļ����������ݣ���ʵ������Է��κ����ݡ�
	int epollfd = epoll_create(1024); //���� epoll �ļ���������������-1
	//��������б��� listenfd ���ж�Ӧ�¼�����ʱ�� epoll_wait �Ὣ epoll_event ��䵽 events_in ������, ������ -1
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fanotify_fd, &epev) == -1)
	{
		cout << "fanotify_fd add epoll listen fail, mesg=" << strerror(errno) << endl;
	}
	struct epoll_event events_in[16];
	struct inotify_event* event;
	while (run_flag)
	{
		//�ȴ��¼��� epoll_wait �Ὣ�¼������ events_in ��
		//���� ��õ��¼�����������ʱ��û���κ��¼����� 0 �������� -1 �� timeout����Ϊ-1��ʾ���޵ȴ������뼶��1�����1000����
		int event_count = epoll_wait(epollfd, events_in, sizeof(events_in) - 1, 10);
		if (event_count == -1)
		{
			cout << "epoll_wait get event fail, mesg=" << strerror(errno) << endl;
		}
		fanotify_handle_event(event_count);
	}
}
bool fanotify_handle_event(const int& event_count) //�����¼�
{
	for (int i = 0; i < event_count; i++)      //���������¼�
	{
		string error_str;
		//out->logs << OUTDEBUG << "event_count:" << event_count;
		//ѭ�������¼����Դ�fanotify�ļ���������ȡ
		const struct fanotify_event_metadata* metadata;
		struct fanotify_event_metadata buf[4096];
		ssize_t len;
		char path[PATH_MAX];
		ssize_t path_len;
		char procfd_path[PATH_MAX];
		struct fanotify_response response;

		len = read(fanotify_fd, (void*)&buf, sizeof(buf)); //��һЩ�¼�
		if (len == -1 && errno != EAGAIN)
		{
			cout << "from fanotify_fd read event fail, mesg=" << strerror(errno) << endl;
		}
		if (len <= 0) //����Ƿ��ѵ����������
		{
			break;
		}
		metadata = buf; //ָ�򻺳����еĵ�һ���¼�

		while (FAN_EVENT_OK(metadata, len))  //ѭ���������е������¼�
		{
			if (metadata->vers != FANOTIFY_METADATA_VERSION)  //�������ʱ�ṹ�ͱ���ʱ�ṹ�Ƿ�ƥ��
			{
				cout << "fanotify metadata version does not match, mesg=" << strerror(errno) << endl;
			}
			if (metadata->fd >= 0)  //metadata->fd����FAN_NOFD(ָʾ�������)���ļ�������(�Ǹ�����)��������Ǽ򵥵غ��Զ��������
			{
				snprintf(procfd_path, sizeof(procfd_path), "/proc/self/fd/%d", metadata->fd); //�����ʹ�ӡ�������ļ���·����
				//readlink()�Ὣ����path�ķ����������ݴ洢������buf��ָ���ڴ�ռ䣬���ص����ݲ�����\000���ַ�����β��
				//���Ὣ�ַ������ַ������أ���ʹ�����\000��ü򵥡�������bufsizС�ڷ������ӵ����ݳ��ȣ����������ݻᱻ�ضϣ�
				//��� readlink ��һ������ָ��һ���ļ������Ƿ�������ʱ��readlink �� ��errno Ϊ EINVAL ������ -1�� 
				//readlink()���������open()��read()��close()�����в�����
				path_len = readlink(procfd_path, path, sizeof(path) - 1);
				if (path_len == -1)
				{
					cout << "readlink fail, mesg=" << strerror(errno) << endl;
				}
				path[path_len] = '\0';
				//printf("File %s\n", path);
				close(metadata->fd); //�ر��¼����ļ�������
				fanotify_decision_event(metadata, path);
			}
			metadata = FAN_EVENT_NEXT(metadata, len); //��ȡ��һ���¼�
		}
	}
	return true;
}
bool fanotify_decision_event(const struct fanotify_event_metadata* metadata, const string& path) //�����¼�
{
	string proc_name = get_proc_name(metadata->pid);
	//����matadata
	if (metadata->mask & FAN_ALL_PERM_EVENTS)
	{
		//out->logs << OUTINFO << path << ", ���ں˷�����Ϣ";
		struct fanotify_response response_struct;
		response_struct.fd = metadata->fd;
		if (pid == metadata->pid)
		{
			response_struct.response = FAN_ALLOW;
			cout << "����" << proc_name << "����" << path << endl;
		}
		else
		{
			response_struct.response = FAN_DENY;
			cout << "�ܾ�" << proc_name << "����" << path << endl;
		}

		int ret = write(fanotify_fd, &response_struct, sizeof(response_struct));
		if (ret < 0)
		{
			return false;
		}
	}
	return true;
}
bool start_process_hidden(int argc, char* argv[]) //��ʼ����ָ������
{
	char file_path_temp[32] = { 0 };
	snprintf(file_path_temp, sizeof(file_path_temp), "/proc/%d", pid);
	string file_path = file_path_temp;
	add_watch_unit(file_path_temp);
	for (int i = 1; i < argc; i++)
	{
		char file_path_temp[32] = { 0 };
		snprintf(file_path_temp, sizeof(file_path_temp), "/proc/%d", atoi(argv[i]));
		string file_path = file_path_temp;
		if (access(file_path_temp, F_OK) != 0) //����ļ�������
		{
			cout << file_path_temp << " is not exists, mesg=" << strerror(errno) << endl;
		}
		else
		{
			add_watch_unit(file_path);
			cout << argv[i] << " process hidden success" << endl;
		}
	}
}
bool stop_process_hidden() //ֹͣ����ָ������
{
	run_flag = false;
}

string get_proc_name(const pid_t& pid)
{
	static pid_t static_pid = -1;
	static string static_proc_name = "";
	if (static_pid != pid)
	{
		char* p = NULL;
		int count = 0;
		char chPath[4096] = { 0 };
		char cParam[4096] = { 0 };

		sprintf(cParam, "/proc/%d/exe", pid);
		count = readlink(cParam, chPath, 4096);
		if (count < 0 || count >= 4096)
		{
			cout << pid << ", Current System not Surport Proc, mesg=" << strerror(errno) << endl;
		}
		else
		{
			p = basename(chPath);
			if (p == NULL)
			{
				cout << pid << ", is not exists, mesg=" << strerror(errno) << endl;
			}
		}
		static_pid = pid;
		static_proc_name = chPath;
	}
	return static_proc_name;
}