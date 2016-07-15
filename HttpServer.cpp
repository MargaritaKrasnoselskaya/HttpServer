#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>//for close
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <functional>
//for thread
#include <thread>
#include <functional>
//for string
#include <iostream>
#include <getopt.h>
#include <cerrno>
//O_RDONLY
#include <fcntl.h>
//for socc=kaddr
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
//for htons
#include <arpa/inet.h>
using namespace std;
static string m_host;
static string m_dir;
static uint16_t m_port = 0;
static int m_socket;
static const int BUFSIZE = 1024*10;
void errorExit(const string text)
{
    cerr<<text<<"\n";
    exit(1);
};
inline bool exists_test (const string& name)
{
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}
void GetHeader(const string str, string &strHeader)
{
    strHeader = "HTTP/1.0 "+str+"\r\nContent-Type: text/html\r\n\r\n";
}
void GetConnection(int argc, char **argv)
{
    int opt = -1;
    while( (opt = getopt( argc, argv, "h:p:d:"))!=-1 )
    {
        switch( opt )
        {//getopt analyse
            case 'h':
                m_host = optarg;
                break;
            case 'p':
                m_port = (uint16_t)atoi(optarg);
               // cerr<<port;
                break;
            case 'd':
                m_dir = optarg;
               // cerr<<"dir: "<<m_dir<<"\n";
                break;
 			default:
               cout<<"Options:\n -h defines IP adress\n  -p defines port\n -d defines directory\n";
                exit(1);
        }
    }
    return;
}
void CreateSocket()
{
    int res;
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (m_socket==-1) errorExit("Error by SocketCreating");
    sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family= AF_INET;
    sa.sin_port = htons(m_port);
    unsigned char buf[sizeof(struct in6_addr)];
    inet_pton(AF_INET, m_host.c_str(), buf);//convert to binary form and fill the structure
    sa.sin_addr.s_addr = *((uint32_t*)buf);
    //server.sin_addr.s_addr = INADDR_ANY;
    //Bind
    if( bind(m_socket, (struct sockaddr *)&sa , sizeof(sa))<0)
        errorExit("Error by Binding");
    //listen
    res = listen(m_socket, SOMAXCONN);
    if (res < 0) errorExit("Error by Listening");
}
int ReadFile(const string fileName, string& resStr)
{
    char temp;
    ifstream ifs(fileName);
    if(!ifs) return 0;
    ifs.unsetf(ios::skipws);
    while(ifs >> temp)
    resStr += temp;
    return 1;
}
void ProcessRequest(int sock)
{   //process Server request
    char buf[BUFSIZE]; //string str;
    recv(sock, buf, BUFSIZE, 0);
    char *firstStr; firstStr = strtok(buf, "\n\r");
    string str(firstStr);
     string header=""; string bodycontent=""; string path="";
    size_t len = str.length();
    do
    {
        if (len<9) { GetHeader("500 Internal Server Error", header); break;}
        if (str.substr(0, 4) != "GET " || str.substr(len-8, 8) != " HTTP/1.") { GetHeader("500 Internal Server Error", header); break;}
        size_t pos = str.find("?");
        if (pos == string::npos) path = str.substr(4, len-12);
        else path = str.substr(4, pos-4);

        struct stat buffer;
        path = m_dir+"/"+path;//full file name
        if (stat (path.c_str(), &buffer) != 0)
        {   bodycontent = "There were no such file or resource. But We can tell you a fairytale. Once upon a time...";
            GetHeader("404 File Not Found\r\nContent-length: 0", header);
            break;
        }
        if (ReadFile(path, bodycontent)==0)
        {
            bodycontent = "There were no such file or resource. But We can tell you a fairytale. Once upon a time...";
            GetHeader("404 File Not Found\r\nContent-length: 0", header);
            break;
        }
        else
        {
            //we already have bodycontent
            string szstr = to_string(bodycontent.size());
            string tmp = "200 OK\r\nContent-length: "+szstr;
            GetHeader(tmp, header);
        }
    } while (false);
    string resstr = header+bodycontent;
    send(sock, resstr.c_str(), resstr.size(), 0);
    shutdown(sock, SHUT_RDWR);
    close(sock);
}
void runServer()
{   int socket; //thread th;
    while (true)
    {
        socket = accept(m_socket, 0, 0);
        thread thread(ProcessRequest, socket);
        thread.detach();
    }
}
int main(int argc, char **argv )
{
    if (argc !=7 ) { cout<<"Wrong format. Use -h adress, -p port, -d dir"; return -1; }
    int   pid=fork(); //start Daemon
    if (pid ==-1 ) { cout<<"Can not start Server Process "; exit(1);}
    if (pid > 0)  return 0;
    //close server Console
    close(STDIN_FILENO); open("/dev/null",O_RDONLY);
    close(STDOUT_FILENO); open("/dev/null",O_WRONLY);
   // close(STDERR_FILENO); //dup(1);
    umask(0);
    chdir("/");
    setsid();
    GetConnection(argc, argv);
    CreateSocket();
    runServer();
    return 0;
 }
